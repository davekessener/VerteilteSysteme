package vs.util;

import java.util.HashMap;
import java.util.Map;

import javafx.beans.property.Property;

public final class Properties
{
	private static final Map<String, Property<?>> sProperties = new HashMap<>();
	
	@SuppressWarnings("unchecked")
	public static <T> Property<T> get(String id) { return (Property<T>) sProperties.get(id); }
	
	private Properties( ) { }
}
