package vs.app;

import javafx.stage.Stage;
import vs.app.client.RedaktionComponent;
import vs.app.client.RezeptionComponent;

public class ClientApp extends App
{
	public ClientApp(Stage primary)
	{
		super(primary);
		
		primary.setTitle(TITLE);
		
		addComponent(new RezeptionComponent());
		addComponent(new RedaktionComponent());
	}
	
	private static final String TITLE = "VS - Client";
}
