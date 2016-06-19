<?xml version = "1.0"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns = "http://www.w3.org/1999/xhtml">
   <head>
       <%@ page
        import = "java.io.*"
        import = "java.lang.*"
        import = "java.sql.*"
        import = "java.util.HashMap"
        import = "tw.com.axtronics.jsp.beans.*"

        import = "java.sql.Connection"
        import = "java.sql.DriverManager"
        import = "java.sql.SQLException"
        import = "java.sql.Statement"
        import = "java.sql.ResultSet"
        import = "java.sql.PreparedStatement"
        import = "java.text.DateFormat"
        import = "java.text.SimpleDateFormat"
        import = "java.util.Date" 
 

      %>
      <jsp:useBean id = "readprocess" scope = "request"
         class = "tw.com.axtronics.jsp.beans.ReadProcess" />
   </head>
   <body>
       <%
       String pName = request.getParameter("product");
       String ver = request.getParameter("version");
       
       String beginTime;
       String endTime;
       long timeStartMs, timeEndMs;
       Date timeStart, timeEnd;
       DateFormat df = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss.SSSS");

       if (pName == null)
          pName = "";
       if (ver == null)
          ver = "";
                                                                                         
       if ( pName.equals("") || ver.equals("") )
       {
       %>
       Netperf test
       <p style = "font-size: 1em;"> Enter the product name and version number of the device under test:
       </p>
       <form action="netperfTest.jsp" method="get">
       <table>
           <tr>
               <td>Product name: </td>
               <td>&nbsp;<input type="text" name="product" maxlength="30" length="30"></td>
           </tr>
           <tr>
               <td>Version number: </td>
               <td>&nbsp;<input type="text" name="version" maxlength="30" length="15"></td>
           </tr>
       </table>
       <p>
           <input type="submit" value="Start testing">
       </p>
       </form>
       <%
       } //end if
       else
       {
        int max_id = 0, numRows = 0;
        Connection connection = null;
        Statement statement = null;
        ResultSet resultSet, resultSet_2;
        boolean firstRow;
                                                                                                
        String jdbc_driver = "com.mysql.jdbc.Driver";
        String database_url = "jdbc:mysql://localhost/tomahawkdb";
        String username = "root";
        String password = "666666";
 
       //out.println("string Pcap: (" + Pcap + ")");
       Runtime runtime = Runtime.getRuntime();
       File filePath = new File("/root/workspace/Netperf/");
       String cmd = "";
           
       cmd = "java -classpath ./:../:/root/commons-net-1.4.1/commons-net-1.4.1.jar TelnetNetperf " + pName + " " + ver;
  
       Process proc = runtime.exec(cmd,null,filePath);
       //Process proc = runtime.exec("java -classpath ./:../:/root/commons-net-1.4.1/commons-net-1.4.1.jar TelnetFtest");
       BufferedReader isr = new BufferedReader(new InputStreamReader(proc.getInputStream()));           
       String tmpStr = null;

       while((tmpStr = isr.readLine()) != null){
                //bw.write(tmpStr+"\n");
           System.out.println(tmpStr);
       }
           //bw.close();
       isr.close();

       //readprocess.main(pName, ver, Pcap); 

       try {
           Class.forName( jdbc_driver );
           connection = DriverManager.getConnection( database_url, username, password );
           statement = connection.createStatement();
           resultSet = statement.executeQuery("SELECT * FROM netperf_task_meta_data");
           
           while (resultSet.next())
               numRows++; 
 
           max_id = numRows;
        }
        catch (ClassNotFoundException classNotFound){
            classNotFound.printStackTrace();
            System.exit(1);
        }
        catch (SQLException sqlException){
            sqlException.printStackTrace();
            System.exit(1);
        }
        try {
            statement.close();
            connection.close();
        }
        catch (Exception exception){
            exception.printStackTrace();
            System.exit(1);
        }
 
       %>
       <!-- Done tomahawk testing. -->
            <jsp:forward page="netperfResult.jsp">
                 <jsp:param name = "run_id" value = "<%= max_id %>" />
            </jsp:forward>
       <%
       } //end else 
       %>
   </body>
</html>
