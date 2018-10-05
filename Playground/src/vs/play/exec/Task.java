package vs.play.exec;

import java.io.Serializable;

public interface Task<T extends Serializable> extends Serializable
{
	public abstract T execute( );
}
