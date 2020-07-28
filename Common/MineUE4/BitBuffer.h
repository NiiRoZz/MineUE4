#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

namespace MineUE4
{
	class BitBuffer
	{
	public:
		// defaultSize is in bit
		BitBuffer(uint64_t defaultSize = 8)
		: m_Data(std::malloc((defaultSize + 7) / 8))
		, m_Pos(0)
		, m_Size(defaultSize)
		, m_Read(false)
		{
			assert(m_Data != nullptr);
		}
		
		// Mostly used when you want to read data
		// defaultSize is in bit
		BitBuffer(void* data, uint64_t size)
		: m_Data(data)
		, m_Pos(0)
		, m_Size(size)
		, m_Read(true)
		{
			assert(m_Data != nullptr);
		}

		~BitBuffer()
		{
			if (!m_Read)
				std::free(m_Data);
		}

		void writeBits(void* data, uint64_t nmbBits)
		{
			if (nmbBits <= 0u)
				return;

			const uint64_t numBitsAfterWrite = m_Pos + nmbBits;
			if (numBitsAfterWrite > m_Size)
			{
				const uint64_t newCount = m_Size + numBitsAfterWrite;
				grow(newCount);
			}

			//If we add byte and we are already at a first byte, just copy memory
			if (m_Pos % 8 == 0 && nmbBits % 8 == 0)
			{
				std::memcpy((uint8_t*)m_Data + (m_Pos / 8), data, (nmbBits / 8));
				m_Pos += nmbBits;
			}
			//Write bit per bit inside the buffer
			else
			{
				for (uint64_t currByte = 0; currByte < nmbBits; ++currByte)
				{
					uint8_t currByteData = ((uint8_t*)data)[(currByte / 8)];
					uint8_t relativePosData = currByte % 8;

					uint8_t& currByteBuffer = ((uint8_t*)m_Data)[(m_Pos / 8)];
					uint8_t relativePosBuffer = m_Pos % 8;

					uint8_t valueCopy = ((currByteData >> relativePosData) & 1) << relativePosBuffer;

					currByteBuffer |= valueCopy;
					m_Pos++;
				}
			}
		}

		void writeBytes(void* data, uint64_t nmbBytes)
		{
			if (nmbBytes <= 0u)
				return;

			writeBits(data, nmbBytes * 8);
		}

		bool readBits(void* data, uint64_t nmbBits)
		{
			if (nmbBits <= 0 || m_Pos + nmbBits > m_Size)
				return false;

			if (m_Pos % 8 == 0 && nmbBits % 8 == 0)
			{
				std::memcpy(data, (uint8_t*)m_Data + (m_Pos / 8), (nmbBits / 8));
				m_Pos += nmbBits;
			}
			else
			{
				for (uint64_t currByte = 0; currByte < nmbBits; ++currByte)
				{
					uint8_t& currByteData = ((uint8_t*)data)[(currByte / 8)];
					uint8_t relativePosData = currByte % 8;

					uint8_t currByteBuffer = ((uint8_t*)m_Data)[(m_Pos / 8)];
					uint8_t relativePosBuffer = m_Pos % 8;

					uint8_t valueCopy = ((currByteBuffer >> relativePosBuffer) & 1) << relativePosData;

					currByteData |= valueCopy;

					m_Pos++;
				}
			}

			return true;
		}

		bool readBytes(void* data, uint64_t nmbBytes)
		{
			return readBits(data, nmbBytes * 8);
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
		//nmbBytes should be > than current size of the buffer
		void grow(const uint64_t nmbBits)
		{
			if (nmbBits <= m_Size || nmbBits <= 0u)
				return;

			m_Data = std::realloc(m_Data, ((nmbBits + 7) / 8));
			assert(m_Data != nullptr);

			m_Size = nmbBits;
		}

		//Current bit data
		void* m_Data;

		//Current pos in the buffer in bit
		uint64_t m_Pos;

		//Size of the buffer in bit
		uint64_t m_Size;

		//Detect if we are reading or writing
		bool m_Read;
	};
}