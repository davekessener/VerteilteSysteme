package vs;

import javafx.application.Application;
import javafx.stage.Stage;

import vs.app.App;
import vs.app.ClientApp;

public class Start extends Application
{
	private App mApplication;
	
	public static void main(String[] args)
	{
		launch(args);
	}

	@Override
	public void start(Stage primary) throws Exception
	{
		mApplication = new ClientApp(primary);
		
		mApplication.start();
	}
	
	@Override
	public void stop( )
	{
		System.out.println("Shutting down ...");
		
		mApplication.stop();
	}
}
