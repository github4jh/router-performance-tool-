import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.HashMap;

public class NetperfOutput {
	int recvSocketSize, sendSocketSize, sendMessageSize;

	double elapsedTime, throughput;

	// static BufferedReader in;

	public void parseNetperfResult() {
		try {
			int skipLines = 6;
			String data[];
			String number[];

			BufferedReader in;
			String line;

			number = new String[5];
			in = new BufferedReader(new FileReader(
					"/root/workspace/Netperf/result.dat"));
			for (int i = 0; i < skipLines; i++)
				in.readLine();

			line = in.readLine();

			System.out.println("line read: " + line);

			in.close();

			data = line.split("\\p{Space}");
			System.out.println("data.length = " + data.length);

			int index = 0;

			for (int i = 0; i < data.length; i++) {
				if (!(data[i].equals(""))) {
					number[index] = data[i];
					index++;
				}
			}

			System.out.println("number.lentgh = " + number.length);

			for (int r = 0; r < number.length; r++)
				System.out.println("number[" + r + "] = " + number[r]);

			this.recvSocketSize = Integer.parseInt(number[0]);
			this.sendSocketSize = Integer.parseInt(number[1]);
			this.sendMessageSize = Integer.parseInt(number[2]);
			this.elapsedTime = Double.parseDouble(number[3]);
			this.throughput = Double.parseDouble(number[4]);
			/*
			 * System.out.println(""); System.out.println("recvSocketSize:" +
			 * this.recvSocketSize); System.out.println("sendSocketSize:" +
			 * this.sendSocketSize); System.out.println("sendMessageSize:" +
			 * this.sendMessageSize); System.out.println("elapsedTime:" +
			 * this.elapsedTime); System.out.println("throughput:" +
			 * this.throughput); System.out.println("");
			 */
			insertToTable();
		} catch (FileNotFoundException e) {
			System.err.println("File not found exception");
		} catch (IOException i) {
			System.err.println("file reading exception");
		}
	}

	public void insertToTable() {
		Connection connection = null;
		Statement statement = null;
		int count;

		String jdbc_driver = "com.mysql.jdbc.Driver";
		String database_url = "jdbc:mysql://localhost/tomahawkdb";
		String username = "root";
		String password = "666666";

		try {
			Class.forName(jdbc_driver);
			connection = DriverManager.getConnection(database_url, username,
					password);
			statement = connection.createStatement();
			PreparedStatement prepareStmt;
			
			prepareStmt = connection
				.prepareStatement("INSERT INTO netperf (recvSocketSize, sendSocketSize, sendMessageSize, elapsedTime, throughPut) VALUES ("
							+ this.recvSocketSize
							+ ", "
							+ this.sendSocketSize
							+ ", "
							+ this.sendMessageSize
							+ ", "
							+ this.elapsedTime + ", " + this.throughput + ")");
			
			//prepareStmt = connection
			//.prepareStatement("INSERT INTO netperf (recvSocketSize, sendSocketSize, sendMessageSize, elapsedTime, throughPut) VALUES (123, 123, 123, 1.0, 2.0)");

			count = prepareStmt.executeUpdate();
			prepareStmt.close();
			System.out
					.println(count + " rows were inserted into table netperf");
		} catch (ClassNotFoundException classNotFound) {
			classNotFound.printStackTrace();
			System.exit(1);
		} catch (SQLException sqlException) {
			sqlException.printStackTrace();
			System.exit(1);
		}
		try {
			statement.close();
			connection.close();
		} catch (Exception exception) {
			exception.printStackTrace();
			System.exit(1);
		}
	}

	public static void main() {
		NetperfOutput n = new NetperfOutput();
		n.parseNetperfResult();
	}
}