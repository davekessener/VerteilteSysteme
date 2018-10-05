package vs.app.client;

import javafx.beans.binding.Bindings;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleObjectProperty;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.SingleSelectionModel;
import javafx.scene.control.TextArea;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.text.Font;

public class RezeptionUI
{
	private final BorderPane mRoot;
	private final ObjectProperty<Mode> mMode;
	private final Button mUpdate;
	private final TextArea mText;
	
	public RezeptionUI( )
	{
		mRoot = new BorderPane();
		
		mMode = new SimpleObjectProperty<>(Mode.MANUAL);
		mUpdate = new Button("Request");
		mText = new TextArea();

		ComboBox<Mode> mode = new ComboBox<>();
		SingleSelectionModel<Mode> m = mode.getSelectionModel();
		
		mode.getItems().addAll(Mode.values());
		
		m.select(Mode.MANUAL);
		m.selectedItemProperty().addListener(o -> mMode.setValue(m.getSelectedItem()));

		mUpdate.setMinWidth(80);
		mUpdate.visibleProperty().bind(Bindings.equal(mMode, Mode.MANUAL));
		
		mText.setEditable(false);
		mText.setFont(Font.font("Courier new", 11));
		
		HBox hb = new HBox();
		
		hb.setPadding(new Insets(0, 0, 4, 0));
		hb.setSpacing(4);
		hb.setAlignment(Pos.CENTER_RIGHT);
		
		hb.getChildren().addAll(mUpdate, mode);
		
		mRoot.setTop(hb);
		mRoot.setCenter(mText);
		
		GridPane.setHgrow(mRoot, Priority.ALWAYS);
		GridPane.setVgrow(mRoot, Priority.ALWAYS);
	}
	
	public Node getUI( ) { return mRoot; }
	public Property<Mode> modeProperty( ) { return mMode; }
	public void setOnUpdate(Runnable r) { mUpdate.setOnAction(e -> r.run()); }
	
	public void addMessage(String msg)
	{
		mText.setText(mText.getText() + "\n" + msg);
		mText.setScrollTop(Double.MAX_VALUE);
	}
	
	public static enum Mode
	{
		MANUAL,
		AUTOMATIC
	}
}
