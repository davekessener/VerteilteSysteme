package vs.app.ui;

import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.ButtonType;

public final class QuickAlert
{
	public static void show(AlertType t, String msg, ButtonType ... btns)
	{
		Alert a = new Alert(t, msg, btns);
		
		a.showAndWait();
	}
	
	private QuickAlert( ) { }
}
