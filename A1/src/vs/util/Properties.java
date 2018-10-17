package vs.util;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import javafx.beans.property.Property;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;

public final class Properties
{
	private static final Map<String, Entry<?>> sProperties = new HashMap<>();
	
	public static final String SERVER_FAILURE_TIMEOUT = "server-timeout";
	public static final String CLIENT_ID = "client-id";
	public static final String TRANSMIT_RETRIES = "retries";
	
	public static final class Defaults
	{
		public static final int SERVER_FAILURE_TIMEOUT = 20 * 1000;
		public static final String CLIENT_ID = "127.0.0.1";
		public static final int TRANSMIT_RETRIES = 8;
		
		private Defaults( ) { }
	}
	
	@SuppressWarnings("unchecked")
	public static <T> Property<T> get(String id) { return (Property<T>) sProperties.get(id).property; }
	public static void set(String id, String v) throws ParseException { sProperties.get(id).set(v); }
	
	public static Set<String> all( ) { return sProperties.keySet(); }
	
	private static interface Parser<T> { T parse(String s) throws ParseException; }
	
	private static final class Entry<T>
	{
		public final Property<T> property;
		public final Parser<T> parser;
		
		public Entry(Property<T> property, Parser<T> parser)
		{
			this.property = property;
			this.parser = parser;
		}
		
		public void set(String v) throws ParseException
		{
			property.setValue(parser.parse(v));
		}
	}
	
	private static class IntegerParser implements Parser<Number>
	{
		@Override
		public Number parse(String v) throws ParseException
		{
			try
			{
				return Integer.parseInt(v);
			}
			catch(NumberFormatException e)
			{
				throw new ParseException("Not a number!");
			}
		}
	}
	
	private static class PositiveIntegerParser extends IntegerParser
	{
		@Override
		public Number parse(String v) throws ParseException
		{
			int r = super.parse(v).intValue();
			
			if(r <= 0)
				throw new ParseException("Not a positive int!");
			
			return r;
		}
	}
	
	static
	{
		sProperties.put(SERVER_FAILURE_TIMEOUT, new Entry<>(new SimpleIntegerProperty(Defaults.SERVER_FAILURE_TIMEOUT), new PositiveIntegerParser()));
		sProperties.put(CLIENT_ID, new Entry<>(new SimpleStringProperty(Defaults.CLIENT_ID), s -> s));
		sProperties.put(TRANSMIT_RETRIES, new Entry<>(new SimpleIntegerProperty(Defaults.TRANSMIT_RETRIES), new PositiveIntegerParser()));
	}
	
	private Properties( ) { }
}
