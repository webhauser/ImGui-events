#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

const int MAX_CIRCBUF_SIZE = 64;

template <class T>
class CircularBuffer {
private:
	int m_head;
	int m_count;
	int max_size;
	T m_buffer[MAX_CIRCBUF_SIZE];

public:
	
	CircularBuffer() : 
		m_head(0), 
		m_count(0),
		max_size(MAX_CIRCBUF_SIZE) {
	}
	~CircularBuffer() {}
	
	void clear() {
		m_head = 0;
		m_count = 0;
	}
	
	int capacity() const {
		return max_size;
	}
	
	int size() const {
		return m_count;
	}
	
	const T& elem(const int index) const {
		if (index >= m_count) {
			throw "CircularBuffer::elem(): index is out of bounds";
		}
		return m_buffer[(m_head + index) % max_size];
	}
	
	// beszuras a vegere
	void insert(const T& value) {
		if (m_count < max_size) {
			if(m_head==0) {
				m_buffer[m_count] = value;
			}
			else {
				m_buffer[(m_head + m_count) % max_size] = value;
			}
			m_count++;
		}
		else {
			m_buffer[m_head] = value;
			m_head = ((m_head + 1) % max_size);
		}
	}
	
	void removeFirst() {
		if(m_count > 0) {
			m_head = ((m_head + 1) % max_size);
			m_count--;
		}
	}
}; 

#endif