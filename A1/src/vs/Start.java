package vs;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.Enumeration;

import dave.arguments.Arguments;
import dave.arguments.Option;
import dave.arguments.Option.OptionBuilder;
import dave.arguments.Parser;
import dave.util.ShutdownService;
import dave.util.TransformingConsumer;
import dave.util.Utils;
import dave.util.ShutdownService.Priority;
import dave.util.log.LogBase;
import dave.util.log.Logger;
import dave.util.log.Severity;
import dave.util.log.SimpleFormatter;
import dave.util.log.Spacer;
import javafx.application.Application;
import javafx.stage.Stage;

import vs.app.App;
import vs.app.client.ConfigurationComponent;
import vs.app.client.RedaktionComponent;
import vs.app.client.RemoteComponent;
import vs.app.client.RezeptionComponent;
import vs.app.client.ServerComponent;
import vs.util.Properties;
import vs.work.ChatEngine;
import vs.work.MessageService;

public class Start extends Application
{
	private App mApplication;
	
	public static void main(String[] args)
	{
		LogBase.INSTANCE.registerSink(e -> true, new TransformingConsumer<>(new Spacer(s -> System.out.println(s), 1500, Utils.repeat("=", 120)), new SimpleFormatter()));
		LogBase.INSTANCE.start();
		
		ShutdownService.INSTANCE.register(Priority.LAST, () -> LogBase.INSTANCE.stop());
		
		Logger.DEFAULT.log("Version %d", VERSION);
		
		launch(args);

		ShutdownService.INSTANCE.shutdown();
		
		System.exit(0);
	}

	@Override
	public void start(Stage primary) throws Exception
	{
		try
		{
			Option o_client = (new OptionBuilder("client")).build();
			Option o_server = (new OptionBuilder("server")).build();
			Option o_host = (new OptionBuilder("host")).setShortcut("h").hasValue(true).setDefault("127.0.0.1").build();
			Option o_port = (new OptionBuilder("port")).setShortcut("p").hasValue(true).setDefault("1099").build();
			Option o_id = (new OptionBuilder("id")).setShortcut("n").hasValue(true).setDefault(getDefaultID()).build();
			Parser parser = new Parser(o_client, o_server, o_host, o_port, o_id);
			Arguments args = parser.parse(getParameters().getRaw().toArray(new String[] {}));
			
			if(!args.hasArgument(o_client) && !args.hasArgument(o_server))
				throw new IllegalArgumentException("Insufficient args given: System has to be either client or server!");
			
			if(args.hasMainArgument())
				throw new IllegalArgumentException("Unrecognized arguments: " + args.getMainArgument());
			
			String host = args.getArgument(o_host);
			int port = Integer.parseInt(args.getArgument(o_port));
			mApplication = new App(primary);
			
			setSecurity(host);

			if(args.hasArgument(o_server))
			{
				ChatEngine chat = new ChatEngine();
				Registry reg = LocateRegistry.createRegistry(port, (ip, p) -> new Socket(ip, p), p -> new ServerSocket(p, 0, InetAddress.getByName(host)));
				MessageService stub = (MessageService) UnicastRemoteObject.exportObject(chat, 0);
				
				reg.rebind(SERVICE_CHAT, stub);
				
				mApplication.addComponent(new ConfigurationComponent(chat));
				mApplication.addComponent(new ServerComponent(chat));
				
				Logger.DEFAULT.log("Starting server and registering with %s:%d", host, port);
				
				primary.setTitle(TITLE + "Server");
			}
			else if(args.hasArgument(o_client))
			{
				String id = args.getArgument(o_id);
				Properties.set(Properties.CLIENT_ID, id);
				
				RemoteComponent<MessageService> c_remote = new RemoteComponent<>(SERVICE_CHAT, host, port);
				RezeptionComponent c_rezeption = new RezeptionComponent(c_remote.serviceProperty());
				RedaktionComponent c_redaktion = new RedaktionComponent(c_remote.serviceProperty());

				mApplication.addComponent(c_remote);
				mApplication.addComponent(c_rezeption);
				mApplication.addComponent(c_redaktion);

				mApplication.connectedProperty().bindBidirectional(c_rezeption.connectedProperty());
				mApplication.connectedProperty().bindBidirectional(c_redaktion.connectedProperty());
				
				mApplication.connectedProperty().set(c_remote.serviceProperty().getValue() != null);
				
				Logger.DEFAULT.log(Severity.INFO, "Starting client %s!", id);
				
				primary.setTitle(TITLE + "Client");
			}
			
			mApplication.start();
		}
		catch(Throwable t)
		{
			System.err.println("Error during startup!");
			System.err.println("usage (server): --server --host {rmi-dir-ip} [--port {rmi-port==1099}]");
			System.err.println("usage (client): --client --id {client-id} --host {rmi-dir-ip} [--port {rmi-port==1099}]");
			
			throw t;
		}
	}

	public static void setSecurity(String host)
	{
		System.setProperty("java.security.policy", "file:./security.policy");
		System.setProperty("java.rmi.server.hostname", host);
	}
	
	private static String getDefaultID( ) throws SocketException
	{
		Enumeration<NetworkInterface> nics = NetworkInterface.getNetworkInterfaces();
		String id = null;
		
		while(nics.hasMoreElements())
		{
			NetworkInterface nic = nics.nextElement();
			
			if(!nic.isUp()) continue;
			if(nic.isLoopback()) continue;
			if(nic.getHardwareAddress() == null) continue;
			if(nic.isPointToPoint()) continue;
			if(nic.isVirtual()) continue;
			
			Enumeration<InetAddress> addresses = nic.getInetAddresses();
			
			while(addresses.hasMoreElements())
			{
				InetAddress a = addresses.nextElement();
				String pot = a.getHostAddress().replaceAll("%.*$", "");
				
				if(id == null || pot.length() < id.length())
				{
					id = pot;
				}
			}
		}
		
		if(id == null)
			throw new IllegalStateException("Could not find a valid network interface!");
		
		return id;
	}
	
	@Override
	public void stop( )
	{
		System.out.println("Shutting down ...");
		
		mApplication.stop();
	}
	
	private static final int VERSION = 2;
	private static final String SERVICE_CHAT = "MessageService";
	private static final String TITLE = "VS Chat - ";
}
