package vs.app.client;

import java.util.function.Function;

import javafx.beans.property.Property;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.layout.GridPane;
import vs.app.ui.FilteredTextField;
import vs.app.ui.IntegerTextField;
import vs.work.ChatEngine;

public class ConfigurationUI
{
	private final GridPane mRoot;
	private final Property<Number> mTimeoutPeriod, mTimeoutCount, mTimeoutDuration;
	private final Property<Number> mCapacity, mForget;
	
	public ConfigurationUI( )
	{
		mRoot = new GridPane();
		
		IntegerTextField to_p = new IntegerTextField(v -> v > 0, ChatEngine.DEF_TIMEOUT_PERIOD);
		IntegerTextField to_n = new IntegerTextField(v -> v > 0, ChatEngine.DEF_TIMEOUT_COUNT);
		IntegerTextField to_d = new IntegerTextField(v -> v > 0, ChatEngine.DEF_TIMEOUT_DURATION);
		IntegerTextField cap = new IntegerTextField(v -> v > 0, ChatEngine.DEF_CAPACITY);
		IntegerTextField forget = new IntegerTextField(v -> v > 0, ChatEngine.DEF_FORGET);
		
		mRoot.setHgap(8);
		mRoot.setVgap(4);
		
		mRoot.addRow(0, new Label("Timeout Period: "), to_p, new Label("Timeout Count: "), to_n, new Label("Timeout Duration: "), to_d);
		mRoot.addRow(1, new Label("Queue capacity: "), cap, new Label("Forget time: "), forget);
		
		Function<FilteredTextField, Property<Number>> create = text -> {
			Property<Number> value = new SimpleIntegerProperty();
			
			text.valueProperty().addListener((ob, o, n) -> value.setValue(Integer.parseInt(n)));
			
			return value;
		};
		
		mTimeoutPeriod = create.apply(to_p);
		mTimeoutCount = create.apply(to_n);
		mTimeoutDuration = create.apply(to_d);
		mCapacity = create.apply(cap);
		mForget = create.apply(forget);
	}

	public Property<Number> timeoutPeriodProperty( ) { return mTimeoutPeriod; }
	public Property<Number> timeoutCountProperty( ) { return mTimeoutCount; }
	public Property<Number> timeoutDurationProperty( ) { return mTimeoutDuration; }
	public Property<Number> capacityProperty( ) { return mCapacity; }
	public Property<Number> forgetProperty( ) { return mForget; }
	
	public Node getUI( ) { return mRoot; }
}
