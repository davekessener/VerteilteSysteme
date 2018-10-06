package vs.app.common;

import dave.util.Actor;
import javafx.beans.property.BooleanProperty;
import javafx.scene.Node;

public interface Component extends Actor
{
	public abstract Node getUI( );
	public abstract BooleanProperty connectedProperty( );
	public default void start( ) { }
	public default void stop( ) { }
}
