package vs.app.client;

import java.util.function.BiConsumer;

import javafx.application.Platform;
import javafx.beans.property.Property;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.HBox;
import vs.app.common.Status;
import vs.app.ui.FilteredTextField;
import vs.app.ui.IntegerTextField;
import vs.util.InetAddressValidator;
import vs.util.Properties;

public class RemoteUI
{
	private final HBox mUI;
	private final Button mUpdate;
	private final TextField mHost, mPort;
	private final Label mRemote, mStatus;
	
	public RemoteUI(String host, int port)
	{
		mUI = new HBox();
		
		mUpdate = new Button("Update");
		mHost = new TextField(host);
		mPort = new IntegerTextField(v -> v > 0, port);
		mRemote = new Label(host + ":" + port);
		mStatus = new Label("n/a");
		
		mUpdate.setMinWidth(80);
		
		mUI.setAlignment(Pos.CENTER_LEFT);
		mUI.setSpacing(4);
		
		Property<String> cid = Properties.get(Properties.CLIENT_ID);
		FilteredTextField t_cid = new FilteredTextField(cid.getValue(), s -> InetAddressValidator.getInstance().isValid(s));
		
		cid.bindBidirectional(t_cid.valueProperty());
		
		mUI.getChildren().addAll(
				new Label("Client ID: "), t_cid,
				new Label("Remote IP: "), mHost,
				new Label("Port: "), mPort,
				mUpdate,
				new Label("Status of"), mRemote, new Label(": "), mStatus);
	}
	
	public Node getUI( )
	{
		return mUI;
	}
	
	public void setRemote(String host, int port)
	{
		Platform.runLater(() -> mRemote.setText(host + ":" + port));
	}
	
	public void setStatus(Status s)
	{
		Platform.runLater(() -> {
			mStatus.setText(s.toString());
			mStatus.setTextFill(s.color);
		});
	}
	
	public void onUpdate(BiConsumer<String, Integer> f)
	{
		mUpdate.setOnAction(e -> f.accept(mHost.getText(), Integer.parseInt(mPort.getText())));
	}
}
