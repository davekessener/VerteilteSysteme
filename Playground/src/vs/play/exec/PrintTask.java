package vs.play.exec;

import java.io.Serializable;

public class PrintTask implements Task<Serializable>
{
	private static final long serialVersionUID = 1874730147545866479L;
	
	private final String mMessage;
	
	public PrintTask(String msg)
	{
		mMessage = msg;
	}
	
	@Override
	public Serializable execute()
	{
		System.out.println(mMessage);
		
		return null;
	}
}
