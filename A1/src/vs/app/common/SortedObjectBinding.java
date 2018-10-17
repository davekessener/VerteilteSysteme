package vs.app.common;

import java.util.Arrays;
import java.util.Comparator;

import javafx.beans.binding.ObjectBinding;
import javafx.beans.value.ObservableValue;

public class SortedObjectBinding<T> extends ObjectBinding<T>
{
	private final ObservableValue<? extends T>[] mObj;
	private final Comparator<T> mCallback;
	
	@SafeVarargs
	public SortedObjectBinding(Comparator<T> f, ObservableValue<? extends T> ... objects)
	{
		mObj = objects;
		mCallback = f;
		
		this.bind(objects);
	}
	
	@Override
	protected T computeValue()
	{
		return Arrays.stream(mObj).map(ObservableValue::getValue).sorted(mCallback).findFirst().get();
	}
}
