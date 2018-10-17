package vs.app.ui;

import javafx.application.Platform;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;

public final class QuickAlert
{
	public static void show(AlertType t, String msg, ButtonType ... btns)
	{
		Platform.runLater(() -> (new Alert(t, msg, btns)).showAndWait());
	}
	
	private QuickAlert( ) { }
}
