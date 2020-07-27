#pragma once

#include <cassert>
#include <cstdint>

namespace MineUE4
{
	class ByteBuffer
	{
	public:
		// defaultSize is in byte
		ByteBuffer(uint64_t defaultSize = 1)
		: m_Data(std::malloc(defaultSize))
		, m_Pos(0)
		, m_Size(defaultSize)
		{
			assert(m_Data != nullptr);
		}
		
		// Mostly used when you want to read data
		// defaultSize is in byte
		ByteBuffer(void* data, uint64_t size)
		: m_Data(data)
		, m_Pos(0)
		, m_Size(size)
		{
			assert(m_Data != nullptr);
		}

		~ByteBuffer()
		{
			std::free(m_Data);
		}

		void writeBits(void* data, uint64_t num)
		{
			writeBytes(data, (num + 7) / 8);
		}

		void writeBytes(void* data, uint64_t num)
		{
			const int64_t numBytesAfterWrite = m_Pos + num;
			if ((uint64_t) numBytesAfterWrite > m_Size)
			{
				const uint64_t newArrayCount = m_Size + numBytesAfterWrite;
				resize(newArrayCount);
			}

			assert((m_Pos + num) <= m_Size);

			std::memcpy((uint8_t*)m_Data + m_Pos, data, num);
			m_Pos += num;
		}

		//nmbBytes should be > than current size of the buffer
		void resize(uint64_t nmbBytes)
		{
			if (nmbBytes <= m_Size)
				return;

			std::realloc(m_Data, nmbBytes);
			m_Size = nmbBytes;
		}

		void* getData() const
		{
			return m_Data;
		}

		uint64_t getSize() const
		{
			return m_Size;
		}

	protected:
		void* m_Data;
		uint64_t m_Pos;
		uint64_t m_Size;
	};
}