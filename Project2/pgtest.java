/**
 * 
 */

/**
 * @author group 22
 *
 */
import java.util.ArrayList;
import java.sql.DriverManager;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;
import java.util.Scanner;
import java.util.Random;


public class pgtest{
    /**
     * @param args
     * @throws Exception
     */
    public static void main(String[] args) throws Exception {
        int seed = 90;
        boolean loop = true;
        String url = "jdbc:postgresql://stampy.cs.wisc.edu/cs564instr?sslfactory=org.postgresql.ssl.NonValidatingFactory&ssl";
        Connection conn = DriverManager.getConnection(url);
         
        Random rand = new Random();
        rand.setSeed(seed);
        
        Scanner prompt = new Scanner(System.in);
        while(loop){
            System.out.println("Enter a query:"); 
            System.out.print("SELECT ");
            String select = "select " + prompt.nextLine();
            System.out.print("FROM ");
            String table = prompt.nextLine();
            select = select + " from " + table;
            int rownum = 0;
            int tableyn = 0;
            String tableName="";
            System.out.println("How many rows should be sampled?");
            rownum = prompt.nextInt();
            System.out.println("Do you want a table (0 for No/ 1 for Yes)");
            tableyn = prompt.nextInt();
            if(tableyn!=0&&tableyn!=1){
                System.out.println("Invalid Input detected. Exiting");
                System.exit(0);//quit program
            } else if (tableyn == 1){
                System.out.print("Enter a name for the table: ");
                prompt.nextLine();
                tableName = prompt.nextLine();
            }
            int t=0;
            int m=0;
            int N=0;
            int n=rownum;
            
            Statement st = conn.createStatement();
            String s = "";
            //System.out.println ("select count(*) from ("+select+")as foo");                         //debug
            ResultSet rs2 = st.executeQuery("select count(*) from ("+select+")as foo");
        
            while (rs2.next()) {
                N = rs2.getInt(1);
            }
            
            
            ResultSet rs = st.executeQuery(select);
            ResultSetMetaData rsmd = rs.getMetaData();
            String tablemeta="";//for create table
            String meta="";//for insert
            //get meta
            for (int i= 1 ; i<=rsmd.getColumnCount(); i++){
                if(tableyn==1){
                    tablemeta += ""+rsmd.getColumnName(i)+" "+rsmd.getColumnTypeName(i)+",";
                    meta += rsmd.getColumnName(i)+",";
                }else{
                    System.out.print(rsmd.getColumnName(i)+"\t");
                }
            }
            
            if(tableyn==1){
                //drops table if duplicate
                try {
                    Statement dropTable = conn.createStatement ();
                    dropTable.executeUpdate ("DROP TABLE " + tableName);
                }
                catch (SQLException e) {
                }
                Statement createTable = conn.createStatement ();
                tablemeta = tablemeta.substring(0,tablemeta.length()-1);//remove last ,
                meta = meta.substring(0,meta.length()-1);//remove last ,
                createTable.executeUpdate ("CREATE TABLE " + tableName +" (" + tablemeta + ")");
                //createTable.close();        
            } else {
                System.out.println();
            }
            
            //Knuth's Algo
            int U=0;
            Statement insertTuple = conn.createStatement();
            String tuple="INSERT into "+tableName+"("+meta+") VALUES (";
            while (rs.next()){
                U = rand.nextInt(2); //step 2
                if((N-t)*U >= n-m){//step 3
                    t++; //step5
                }else{
                    for (int i= 1 ; i<=rsmd.getColumnCount(); i++){
                        //step 4
                        if(tableyn == 0){
                            System.out.print(rs.getString(i)+"\t");
                        } else if(tableyn==1){
                            tuple += rs.getString(i)+",";
                        }
                        
                    }
                    
                    if(tableyn == 0){
                        System.out.println(); 
                    }else if(tableyn == 1){
                        tuple = tuple.substring(0,tuple.length()-1) +");";
                        //System.out.println(tuple);///debug
                        insertTuple.executeUpdate(tuple);
                        tuple = "INSERT into "+tableName+"("+meta+") VALUES (";//refresh
                    }
                    
                    m++;t++;
                    if(m == n) break;
                    
                }
                
            }
            if(n>=N){
                System.out.println("All rows are selected.");
            }
            if(tableyn==1){
                System.out.println("Table created.");
            }

            System.out.println("Do you want to quit(-1)? Any positive integer to continue.");
            int restart = prompt.nextInt();
            if(restart < 0){
                loop = false;
                //close up shop
                rs.close();
                rs2.close();
                insertTuple.close();
                st.close();
            } else {
                System.out.println("Do you want to set the seed?(0 for NO/1 for YES)");
                int setseed = prompt.nextInt();
                if(setseed == 1){
                    System.out.println("Enter seed.");
                    setseed = prompt.nextInt();
                    rand.setSeed(setseed); 
                }
                prompt.nextLine();//absorb nextline
            }

        }
           
        // close connection
        conn.close();
    }
}
