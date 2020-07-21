//环形缓冲区头文件
#ifndef CCycleBuffer_H
#define CCycleBuffer_H
typedef unsigned char uchar;
class CCycleBuffer
{
public:
	bool isFull();
	bool isEmpty();
	void empty();
	int getLength();
	CCycleBuffer(int size);
	virtual~CCycleBuffer();
	int write(uchar* buf, int count);
	int read(uchar* buf, int count);
	int getStart()
	{
		return m_nReadPos;
	}
	int getEnd()
	{
		return m_nWritePos;
	}

private:
	bool m_bEmpty, m_bFull;
	uchar* m_pBuf;
	int m_nBufSize;
	int m_nReadPos;
	int m_nWritePos;
	int test;
};
#endif// CCycleBuffer_H