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
      %>
      <jsp:useBean id = "dbConfig" scope = "request"
         class = "tw.com.axtronics.jsp.beans.DatabaseConfig" />
  
      <title>Netperf Test Result</title>
 
   </head>
   <body>
<%
      Connection dbconn;
      ResultSet results;
      PreparedStatement sql;
      Statement statement = null;
      
      HashMap dbConfData = dbConfig.getConfig(); 

      //database configuration
      String jdbc_driver = (String)dbConfData.get("JDBC_DRIVER");
      String database_url = (String)dbConfData.get("DATABASE_URL");
      String username = (String)dbConfData.get("USERNAME");
      String password = (String)dbConfData.get("PASSWORD"); 

      String pName = request.getParameter("product");
      String ver = request.getParameter("version");
      String testDate = request.getParameter("date");
      String runID = request.getParameter("run_id");
      String pFile[];

      if (pName == null) 
          pName = "";
      if (ver == null)
          ver = ""; 
      if (testDate == null)
          testDate = "";
      if (runID == null)
          runID = "";
 
      try
      {
         Class.forName(jdbc_driver);
         try
	 {
            int RunID = 0, RecvSocketSize=0, SendSocketSize=0, SendMessageSize=0;
            double ElapsedTime=0.0, Throughput=0.0;
            int rowCount = 0, index = 0, max_id = 0;
            String[] productNames = new String[1]; 
            String[] verNumbers = new String[1];
            String[] startDate = new String[1];
            String[] parsedDate = new String[1];
            int unique = 0;
            String sqlCmd;

            sqlCmd = "select distinct product_name from netperf_task_meta_data";
	    dbconn = DriverManager.getConnection(database_url, username, password);
	    sql = dbconn.prepareStatement(sqlCmd);

            results = sql.executeQuery();

            results.beforeFirst();
            while(results.next())
	    {
                rowCount++;
            }

            productNames = new String[rowCount];
            results.beforeFirst();
            //save all unique product_name field values in productNames[]
            while(results.next())
            {
                productNames[index] = results.getString("product_name");
                index++;
            }

            sqlCmd = "";
            sqlCmd = "select distinct version from netperf_task_meta_data";
            sql = dbconn.prepareStatement(sqlCmd);
                                                                                          
            results = sql.executeQuery();
                                                                                          
            results.beforeFirst();
            rowCount = 0;
            while(results.next())
            {
                rowCount++;
            }
                                                                                          
            verNumbers = new String[rowCount];
            results.beforeFirst();
            index = 0;
            //save all unique version field values in verNumbers[]
            while(results.next())
            {
                verNumbers[index] = results.getString("version");
                index++;
            }

            sqlCmd = "";
            sqlCmd = "select distinct start_time from netperf_task_meta_data";
            sql = dbconn.prepareStatement(sqlCmd);
                                                                                          
            results = sql.executeQuery();
                                                                                          
            results.beforeFirst();
            rowCount = 0;
            while(results.next())
            {
                rowCount++;
            }
                                                                                          
            startDate = new String[rowCount];
            parsedDate = new String[rowCount];
            results.beforeFirst();
            index = 0;
            //save all unique start_time field values in startDate[]
            while(results.next())
            {
                startDate[index] = results.getString("start_time");
                index++;
            }
/*
            sqlCmd = "";
            sqlCmd = "SELECT MAX(run_id) FROM netperf";
            sql = dbconn.prepareStatement(sqlCmd);
                                                                                          
            results = sql.executeQuery();
            max_id = results.getInt("run_id");

            dbconn = DriverManager.getConnection( database_url, username, password );
           statement = dbconn.createStatement();
           results = statement.executeQuery("SELECT MAX(run_id) FROM netperf");
           max_id = results.getInt("run_id");
*/         
%>
      <a href="netperfTest.jsp?product=&version=">netperf test page</a> 
      <form action="netperfResult.jsp" method="get">
      <p>Enter the product name, version number or date to search for:</p>
          <table>
             <tr>
               <td>Product name:</td>
               <!-- <td><input type="text" name="product"/></td> -->
               <td><select name="product">
                   <option value="">
        <%
                
               for (int t = 0; t < productNames.length; t++)
               {
        %>
                   <option value="<%= productNames[t] %>"><%= productNames[t] %>
               <!--
                   <option value="UTM6000c">UTM6000c
                   <option value="Wireless-Guard">Wireless-Guard
                   <option value="ISG">ISG
                   </select>
               -->
        <%
               }
        %>
                   </select>
               </td>
<!--
                   <option value="UTM6000c">UTM6000c
                   <option value="Wireless-Guard">Wireless-Guard
                   <option value="ISG">ISG
                   </select> 
               </td>
      -->
             </tr>
             <tr>
               <td> Version number:</td>
               <td><select name="version">
                   <option value="">
        <%
               for (int r = 0; r < verNumbers.length; r++)
               {
        %>
                   <option value="<%= verNumbers[r] %>"><%= verNumbers[r] %>
        <%
               }
        %>
                   </select>
               </td>
               <!--<td><input type="text" name="version"/></td>-->
             </tr>
             <tr>
               <td> Date: </td>
               <td><select name="date">
                   <option value="">
        <%
               //only put year/month/date from the start_time field in drop down menu
               for (int e = 0; e < startDate.length; e++)
               {
                   String tmp;
                   int space = 0, repeat = 0;
                                                      
                   //find the space that separates year/month/date from hr:min:sec.msec      
                   space = startDate[e].indexOf(' '); 
                   if (space != -1)
                   {
                       //save year/month/date in tmp
                       tmp = startDate[e].substring(0, space);
                       //check if this particular year/month/date combination is already in 
                       //drop down menu
                       for (int q = 0; q < unique; q++)
                       {
                           if (tmp.compareTo(parsedDate[q]) == 0)
                           {
                               repeat = 1;
                               break;
                           }
                       }
                       //not in drop down menu; parsedDate contains all unique year/month/date
                       //combination
                       if (repeat == 0)
                       {
                           parsedDate[unique] = tmp;
                           unique++;
                                                                                          
        %>
                           <option value="<%= tmp %>"><%= tmp %>
        <%
                       }
                   }
                                                                                          
               }
        %>
                   </select>
               </td>
               <!--<td> <input type="text" name="date"/></td>-->
             </tr>
             <tr>
               <td colspan="2"><input type="submit" value="Search"/></td>
             </tr>
          </table>
      </form>


   <% 
      //search on version number alone is not allowed 
      if ( pName.equals("") && testDate.equals("") && !(ver.equals("")) )
      {
%>

     <p style = "font-color: red;">Version number can not be searched by itself!</p>
  <%
      }
      else
      {
         if ( !(pName.equals("")) ) 
         {   
%>
            product name: <%= pName %><br>
   <%
         }
         if ( !(testDate.equals("")) )
         { 
   %>
            date: <%= testDate %><br>
   <%
         }
         if ( !(ver.equals("")) )
         {
   %>   
            version: <%= ver %><br>
<%
         }
         if ( !(runID.equals("")) )
         {
   %>   
            run_id: <%= runID %><br>
<%
         }
 
            sqlCmd = ""; 
            String cmdPrefix = "SELECT T.* from netperf as T LEFT JOIN netperf_task_meta_data as D ON ";
            //form the query command according to search criteria selected
            if ( !(runID.equals("")) )
            {
                sqlCmd = "SELECT * FROM netperf where run_id = '" + runID + "'";
            }  
            else if( pName.equals("") && testDate.equals("") && ver.equals("") )      
            {
	       sqlCmd = "SELECT * FROM netperf";
            }
            else if (!(pName.equals(""))  && !(ver.equals(""))  && testDate.equals(""))
            {
               sqlCmd = cmdPrefix + " D.product_name = '" + pName + "' AND D.version = '" + ver + "' WHERE D.run_id = T.run_id";
            }
            else if (!(pName.equals(""))  && (ver.equals(""))  && testDate.equals(""))
            {
               sqlCmd = cmdPrefix + " D.product_name = '" + pName + "' WHERE D.run_id = T.run_id";
            }
            else if ( (pName.equals(""))  && (ver.equals(""))  && !(testDate.equals("")) )
            {
               sqlCmd = cmdPrefix + " D.start_time LIKE '" + testDate + "%' WHERE D.run_id = T.run_id";
            }
            else if ( (pName.equals(""))  && !(ver.equals(""))  && !(testDate.equals("")) )
            {
               sqlCmd = cmdPrefix + " D.start_time LIKE '" + testDate + "%' AND D.version = '" + ver + "' WHERE D.run_id = T.run_id";
            }
            else if ( !(pName.equals(""))  && !(ver.equals(""))  && !(testDate.equals("")) )
            {
               sqlCmd = cmdPrefix + " D.start_time LIKE '" + testDate + "%' AND D.version = '" + ver + "' AND D.product_name = '" + pName  + "' WHERE D.run_id = T.run_id";
            }
            else if ( !(pName.equals(""))  && (ver.equals(""))  && !(testDate.equals("")) )
            {
               sqlCmd = cmdPrefix + " D.start_time LIKE '" + testDate + "%' AND D.product_name = '" + pName  + "' WHERE D.run_id = T.run_id";
            }
           
           %>
                <!-- check if sql command is correct -->
                <!-- sqlCmd = <%= sqlCmd %><br> -->
           <%
	    //dbconn = DriverManager.getConnection(database_url, username, password);

	    sql = dbconn.prepareStatement(sqlCmd);

	    results = sql.executeQuery();

            results.beforeFirst();
            rowCount = 0;
            while(results.next())
	    {
                rowCount++;
            }
            if (rowCount > 0)
            {

  %>
              matches found for the above search criteria.<br> 
              <table border=2>
                 <thead>
                    <tr>
                     <th>Run id</th>
                     <th>Receive socket size</th>
                     <th>Send socket size</th>
                     <th>Send message size</th>
                     <th>Elapsed time</th>
                     <th>Throughput</th>
                    </tr>
                 </thead>
          
     <%
           
              results.beforeFirst();
              while(results.next())
	      {
                  RunID = results.getInt("run_id");
                  RecvSocketSize = results.getInt("recvSocketSize");
                  SendSocketSize = results.getInt("sendSocketSize");
                  SendMessageSize = results.getInt("sendMessageSize");
                  ElapsedTime = results.getDouble("elapsedTime");
                  Throughput = results.getDouble("throughPut");

     %> 

                 <tr>
                  <td><%= RunID %></td>
                  <td><%= RecvSocketSize %> &nbsp; bytes</td>
                  <td><%= SendSocketSize %> &nbsp; bytes</td>
                  <td><%= SendMessageSize %> &nbsp; bytes</td>
                  <td><%= ElapsedTime %> &nbsp; seconds</td>
                  <td><%= Throughput %> &nbsp; MBits/sec</td>
                 </tr>

    <%            
	      } //end while
    %>
            </table>
<%

           } //end if
           else
           {
              out.println("<br> no matches found!");
           }
         } //end else 

         }
         catch (SQLException s)
         {
	    out.println("SQL Error");
	 }
      }  
      catch (ClassNotFoundException err)
      {
 	 out.println("Class loading error");
      }    
  %> 
   </body>
</html>
