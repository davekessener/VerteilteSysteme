package vs.app.client;

import java.util.function.BiConsumer;

import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.HBox;
import vs.app.ui.FilteredTextField;
import vs.app.ui.IntegerTextField;
import vs.util.InetAddressValidator;

public class RemoteUI
{
	private final HBox mUI;
	private final Button mUpdate;
	private final TextField mHost, mPort;
	
	public RemoteUI(String host, int port)
	{
		mUI = new HBox();
		
		mUpdate = new Button("Update");
		mHost = new FilteredTextField(host, this::isValid);
		mPort = new IntegerTextField(v -> v > 0, port);
		
		mUpdate.setMinWidth(80);
		
		mUI.setAlignment(Pos.CENTER_LEFT);
		mUI.setSpacing(4);
		
		mUI.getChildren().addAll(new Label("Remote IP: "), mHost, new Label("Port: "), mPort, mUpdate);
	}
	
	public Node getUI( )
	{
		return mUI;
	}
	
	public void onUpdate(BiConsumer<String, Integer> f)
	{
		mUpdate.setOnAction(e -> f.accept(mHost.getText(), Integer.parseInt(mPort.getText())));
	}
	
	private boolean isValid(String txt)
	{
		return InetAddressValidator.getInstance().isValid(txt);
	}
}
