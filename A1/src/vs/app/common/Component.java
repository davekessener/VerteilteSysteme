package vs.app.common;

import dave.util.Actor;
import javafx.scene.Node;

public interface Component extends Actor
{
	public abstract Node getUI( );
	public default void start( ) { }
	public default void stop( ) { }
}
