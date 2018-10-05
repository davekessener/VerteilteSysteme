package vs.play;

import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.Arrays;
import java.util.Scanner;
import java.util.stream.Collectors;

import dave.arguments.Arguments;
import dave.arguments.Option;
import dave.arguments.Option.OptionBuilder;
import dave.arguments.ParseException;
import dave.arguments.Parser;
import dave.util.ShutdownService;
import dave.util.ShutdownService.Priority;
import dave.util.TransformingConsumer;
import dave.util.Utils;
import dave.util.log.LogBase;
import dave.util.log.Logger;
import dave.util.log.SimpleFormatter;
import dave.util.log.Spacer;
import vs.play.exec.BasicExecutor;
import vs.play.exec.PrintTask;
import vs.play.exec.RemoteExecutor;

public class Start
{
	private static RemoteExecutor engine = new BasicExecutor();
	
	public static void main(String[] args) throws RemoteException, NotBoundException, ParseException
	{
		LogBase.INSTANCE.registerSink(e -> true, new TransformingConsumer<>(new Spacer(s -> System.out.println(s), 1500, Utils.repeat("=", 120)), new SimpleFormatter()));
		LogBase.INSTANCE.start();
		
		ShutdownService.INSTANCE.register(Priority.LAST, () -> LogBase.INSTANCE.stop());
		
		Logger.DEFAULT.log("Version %d", VERSION);
		
		Option o_host = (new OptionBuilder("host")).setShortcut("h").setDefault(LOCALHOST).build();
		Option o_port = (new OptionBuilder("port")).setShortcut("p").setDefault("" + PORT).build();
		Option o_msg = (new OptionBuilder("message")).setShortcut("m").setDefault(MESSAGE).build();
		Parser p = new Parser(o_host, o_port, o_msg);
		Arguments a = p.parse(args);
		
		String host = a.getArgument(o_host);
		int port = Integer.parseInt(a.getArgument(o_port));
		boolean run_client = false, run_server = false;

		setSecurity(host);
		
		if(a.hasMainArgument())
		{
			if(a.getMainArgument().equals("listen"))
			{
				run_server = true;
			}
			else if(a.getMainArgument().equals("execute"))
			{
				run_client = true;
			}
			else
			{
				throw new IllegalArgumentException(Arrays.stream(args).collect(Collectors.joining(" ")));
			}
		}
		else
		{
			run_server = run_client = true;
		}
		
		if(run_server)
		{
			Logger.DEFAULT.log("Starting server on %s:%d", host, port);
			
			runServer(host, port);
		}
		
		if(run_client)
		{
			String msg = a.getArgument(o_msg);
			
			Logger.DEFAULT.log("Sending message '%s' to %s:%d", msg, host, port);
			
			runClient(host, port, msg);
		}
		
		Logger.DEFAULT.log("[DONE]");
		
		if(run_server)
		{
			for(Scanner in = new Scanner(System.in) ; true ;)
			{
				System.out.print("> ");
				
				String line = in.nextLine();
				
				if(line == null) break;
				if(line.startsWith("q")) break;
			}
		}
		
		ShutdownService.INSTANCE.shutdown();
		
		System.exit(0);
	}
	
	public static void runServer(String host, int port) throws RemoteException
	{
		Registry reg = LocateRegistry.createRegistry(port, (ip, p) -> new Socket(ip, p), p -> new ServerSocket(p, 0, InetAddress.getByName(host)));
		RemoteExecutor stub = (RemoteExecutor) UnicastRemoteObject.exportObject(engine, 0);
		
		reg.rebind(SERVICE, stub);
	}
	
	public static void runClient(String host, int port, String msg) throws RemoteException, NotBoundException
	{
		Registry reg = LocateRegistry.getRegistry(host, port);
		RemoteExecutor remote_engine = (RemoteExecutor) reg.lookup(SERVICE);
		
		remote_engine.run(new PrintTask(msg));
	}
	
	public static void setSecurity(String host)
	{
		System.setProperty("java.security.policy", "file:./security.policy");
		System.setProperty("java.rmi.server.hostname", host);
	}
	
	private static final String SERVICE = "RemoteExecutionEngine";
	private static final String MESSAGE = "Hello, World!";
	private static final String LOCALHOST = "127.0.0.1";
	private static final int PORT = 1099;
	private static final int VERSION = 5;
}
