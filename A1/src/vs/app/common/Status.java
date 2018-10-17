package vs.app.common;

import javafx.scene.paint.Color;

public enum Status
{
	CONNECTED(Color.GREEN),
	SEARCHING(Color.ORANGE),
	DISCONNECTED(Color.RED);
	
	public final Color color;
	
	private Status(Color color)
	{
		this.color = color;
	}
}
