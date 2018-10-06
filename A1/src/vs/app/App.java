package vs.app;

import java.util.ArrayList;
import java.util.List;

import dave.util.Actor;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;
import vs.app.common.Component;
import vs.app.ui.AppUI;

public class App implements Actor
{
	private final Stage mPrimary;
	private final AppUI mUI;
	private final List<Component> mComponents;
	private final BooleanProperty mConnected;
	
	public App(Stage primary)
	{
		mPrimary = primary;
		mUI = new AppUI();
		mComponents = new ArrayList<>();
		mConnected = new SimpleBooleanProperty(true);
		
		AnchorPane root = new AnchorPane();
		Node ui = mUI.getUI();
		Scene s = new Scene(root, 1280, 720);
		
		root.getChildren().add(ui);
		
		AnchorPane.setLeftAnchor(ui, 0d);
		AnchorPane.setRightAnchor(ui, 0d);
		AnchorPane.setTopAnchor(ui, 0d);
		AnchorPane.setBottomAnchor(ui, 0d);
		
		mPrimary.setScene(s);
		
		mPrimary.show();
	}
	
	public void addComponent(Component c)
	{
		mComponents.add(c);
		mUI.add(c.getUI());
		mConnected.bindBidirectional(c.connectedProperty());
	}

	@Override
	public void start()
	{
		mComponents.forEach(Component::start);
	}

	@Override
	public void stop()
	{
		mComponents.forEach(Component::stop);
	}
}
