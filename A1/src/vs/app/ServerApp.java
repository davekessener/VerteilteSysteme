package vs.app;

import javafx.stage.Stage;

public class ServerApp extends App
{
	public ServerApp(Stage primary)
	{
		super(primary);
		
		primary.setTitle(TITLE);
	}
	
	private static final String TITLE = "VS - Server";
}
