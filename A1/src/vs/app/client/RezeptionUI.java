package vs.app.client;

import java.util.Arrays;

import javafx.application.Platform;
import javafx.beans.property.BooleanProperty;
import javafx.beans.property.Property;
import javafx.beans.property.SimpleBooleanProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.SingleSelectionModel;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import vs.app.ui.IntegerTextField;

public class RezeptionUI
{
	private final BorderPane mRoot;
	private final Button mUpdate;
	private final TextArea mText;
	private final Property<Number> mFreq;
	private final BooleanProperty mActive, mConnected;
	
	public RezeptionUI( )
	{
		mRoot = new BorderPane();
		
		mUpdate = new Button("Request");
		mText = new TextArea();
		mFreq = new SimpleIntegerProperty(DEF_FREQ);
		mActive = new SimpleBooleanProperty(false);
		mConnected = new SimpleBooleanProperty(false);
		
		TextField fr = new IntegerTextField(v -> v > 0, DEF_FREQ);
		ComboBox<Mode> mode = new ComboBox<>();
		SingleSelectionModel<Mode> m = mode.getSelectionModel();
		HBox manual = new HBox(), automatic = new HBox();
		GridPane top = new GridPane();
		Button active = new Button(BTN_START);
		
		mode.getItems().addAll(Mode.values());
		
		m.select(Mode.MANUAL);
		
		mUpdate.setMinWidth(80);
		active.setMinWidth(80);
		
		mText.setEditable(false);
		mText.setFont(Font.font("Courier new", 12));
		
		Arrays.stream(new HBox[] { manual, automatic }).forEach(hb -> {
			hb.setSpacing(4);
			hb.setAlignment(Pos.CENTER_RIGHT);
		});
		
		manual.getChildren().addAll(mUpdate);
		automatic.getChildren().addAll(new Label("Period"), fr, active);
		
		Label s = new Label();
		
		mConnected.addListener((ob, o, n) -> Platform.runLater(() -> {
			if(n)
			{
				s.setText("Connected");
				s.setTextFill(Color.GREEN);
			}
			else
			{
				s.setText("Disconnected");
				s.setTextFill(Color.RED);
			}
		}));
		
		mConnected.set(true);
		
		top.add(new Label("Status: "), 0, 0);
		top.add(s, 1, 0);
		top.add(manual, 2, 0);
		top.add(mode, 3, 0);
		
		top.setAlignment(Pos.CENTER);
		top.setHgap(4);
		top.setPadding(new Insets(0, 0, 4, 0));

		GridPane.setHgrow(manual, Priority.ALWAYS);
		GridPane.setHgrow(automatic, Priority.ALWAYS);
		
		mRoot.setTop(top);
		mRoot.setCenter(mText);
		
		GridPane.setHgrow(mRoot, Priority.ALWAYS);
		GridPane.setVgrow(mRoot, Priority.ALWAYS);
		
		active.setOnAction(e -> mActive.set(!mActive.get()));
		
		mActive.addListener(o -> active.setText(mActive.get() ? BTN_STOP : BTN_START));
		fr.disableProperty().bind(mActive);

		m.selectedItemProperty().addListener(o -> {
			if(m.getSelectedItem() == Mode.MANUAL)
			{
				top.getChildren().remove(automatic);
				top.add(manual, 1, 0);
				mActive.set(false);
			}
			else
			{
				top.getChildren().remove(manual);
				top.add(automatic, 1, 0);
			}
		});
	}
	
	public Node getUI( ) { return mRoot; }
	public int getFrequency( ) { return mFreq.getValue().intValue(); }
	public BooleanProperty activeProperty( ) { return mActive; }
	public BooleanProperty connectedProperty( ) { return mConnected; }
	public void setOnUpdate(Runnable r) { mUpdate.setOnAction(e -> r.run()); }
	
	public void addMessage(String msg)
	{
		mText.setText(mText.getText() + msg + "\n");
		mText.setScrollTop(Double.MAX_VALUE);
	}
	
	public static enum Mode
	{
		MANUAL,
		AUTOMATIC
	}
	
	private static final int DEF_FREQ = 50;
	private static final String BTN_START = "Start";
	private static final String BTN_STOP = "Stop";
}
