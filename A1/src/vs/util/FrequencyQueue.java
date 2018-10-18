package vs.util;

/**
 *  Utility class used to track the
 *  frequency of messages sent.
 *  next() is called with every message;
 *  its return value determines if the
 *  message should be accepted (true) or
 *  if it constitutes spam (false)
 */
public class FrequencyQueue
{
	private final long[] mBuf;
	private final int mTimeout;
	private int mIdx;
	
	public FrequencyQueue(int d, int n)
	{
		mBuf = new long[n];
		mTimeout = d;
		
		reset();
	}
	
	public boolean next( )
	{
		long t = System.currentTimeMillis();
		boolean f = (mBuf[mIdx] == -1 || (t - mBuf[mIdx]) >= mTimeout);
		
		if(f)
		{
			mBuf[mIdx] = t;
			mIdx = (mIdx + 1) % mBuf.length;
		}
		
		return f;
	}
	
	public void reset( )
	{
		for(int i = 0 ; i < mBuf.length ; ++i)
		{
			mBuf[i] = -1;
		}
		
		mIdx = 0;
	}
}
