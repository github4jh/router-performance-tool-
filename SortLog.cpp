#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h> 


void sortFile(char* file, int count);
int parseLine(char* line, long& a, long& b);
long str_to_int(char * str);
void selectionSort(struct LineData* data, int count);
 
struct LineData
{
	long first_number;
	long second_number;
	int line_number;
};

int main()
{
	char line[5000];
	int lineCount = 0;
	char * pch;
  
  
	fstream infile("input.txt", ios::in);
	fstream temp("temp.txt", ios::in | ios::out);
	

    while (!infile.eof())
	{
		infile.getline (line,5000);
		if (strlen(line) != 0)
		{
			lineCount++;
			pch=strchr(line,'(');
			temp<< pch << "\n";
		}
	}

    //cout << "total lines: " << lineCount << endl; 

	if (infile.is_open())
	{
    	infile.close();
	}

	
    if (temp.is_open())
	{
    	temp.close();
	}

	if (lineCount >= 2)
		sortFile("temp.txt", lineCount);
    
    cout << "Done sorting!" << endl;

	return 0;
}

void sortFile(char* file, int count)
{	
	int index = 0, i = 0, j=0, k=0;
    char line[5000];
	int returnValue = 0;
	struct LineData* data;
    int currentline = 0;

    data = new struct LineData[count];			

	fstream infile(file, ios::in);
    fstream outfile("sorted.txt", ios::out);
	
     
	
    //file.seekg(0, ios::beg);
	for (i=0; i<count; i++)
	{
		infile.getline (line,5000);
		currentline++;
		returnValue = parseLine(line, data[i].first_number, data[i].second_number);
		data[i].line_number = currentline;
	    if (returnValue == -1)
		{
			if (infile.is_open())
			{
    			infile.close();
			}

			if (outfile.is_open())
			{
    			outfile.close();
			}
			return;
		}
	}


	cout << endl;
    /*
	cout << "Before sorting." << endl;

	for (j=0;j<count;j++)
	{
		cout << data[j].first_number << "  " << data[j].second_number << "  " << data[j].line_number << endl;			  
	}    

    cout << "After sorting." << endl;

	for (j=0;j<count;j++)
	{
		cout << data[j].first_number << "  " << data[j].second_number << "  " << data[j].line_number << endl;			  
	}

	*/

	selectionSort(data, count);

    for (j=0;j<count;j++)
	{
		infile.seekg(0, ios::beg);  //set get pointer to the beginning of the file

		for(k=0;k<data[j].line_number;k++)
			infile.getline (line,5000);

        outfile<< line << "\n"; 
	}


	if (infile.is_open())
	{
    	infile.close();
	}

	if (outfile.is_open())
	{
    	outfile.close();
	}
	

}


int parseLine(char* line, long& a, long& b)
{					
    char * pch;
	char temp_a[7], temp_b[7];
	
	//cout << "Looking for the ')' character." << endl;
	pch=strchr(line,')');
	
	//cout << "found at " << pch-line+1 << endl; 
	
	if ((pch-line+1) == 14)
	{
        //cout << "Looking for the ':' character." << endl;
	    pch=strchr(line,':');

		if ((pch-line+1) != 4)
		{
			cout << "wrong format. can not sort file." << endl;
			return -1;
        }
        else
		{
            //cout << "found at " << pch-line+1 << endl;
			
			temp_a[0] = line[1];
			temp_a[1] = line[2];
			temp_a[2] = line[4];
			temp_a[3] = line[5];
			temp_a[4] = '\0';
		    a = str_to_int(temp_a);
			//cout << "a = " << a << endl;
			         

			temp_b[0] = line[7];
			temp_b[1] = line[8];
			temp_b[2] = line[9];
			temp_b[3] = line[10];
			temp_b[4] = line[11];
			temp_b[5] = line[12];
			temp_b[6] = '\0';
		    b = str_to_int(temp_b);
			//cout << "b = " << b << endl;
		}

		return 0;
	}
	else if ((pch-line+1) == 17)
	{
        //cout << "Looking for the ':' character." << endl;
	    pch=strchr(line,':');

		if ((pch-line+1) != 4)
		{
			cout << "wrong format. can not sort file." << endl;
			return -1;
        }
        else
		{
            //cout << "found at " << pch-line+1 << endl;
			
			temp_a[0] = line[1];
			temp_a[1] = line[2];
			temp_a[2] = line[4];
			temp_a[3] = line[5];
			temp_a[4] = line[7];
			temp_a[5] = line[8];
			temp_a[6] = '\0';
		    a = str_to_int(temp_a);
			//cout << "a = " << a << endl;
			         

			temp_b[0] = line[10];
			temp_b[1] = line[11];
			temp_b[2] = line[12];
			temp_b[3] = line[13];
			temp_b[4] = line[14];
			temp_b[5] = line[15];
			temp_b[6] = '\0';
		    b = str_to_int(temp_b);
			//cout << "b = " << b << endl;
		}



		return 0;
	}
	else
	{
		cout << "wrong format. can not sort file." << endl;
		return -1;
	}

	

}

void selectionSort(struct LineData* data, int count)
{
	int i, j, min;
	struct LineData tmp;

	for(i=0; i < count -1; i++)
	{
		for (j=i, min=i+1; j<count; j++)
		{
			if(data[j].first_number<data[min].first_number)
				min=j;
			else if (data[j].first_number==data[min].first_number)
			{
				if  (data[j].second_number<data[min].second_number)
					min=j;
			}
		}
		tmp.first_number = data[i].first_number;
		tmp.second_number = data[i].second_number;
		tmp.line_number = data[i].line_number;
		data[i].first_number = data[min].first_number;
		data[i].second_number = data[min].second_number;
		data[i].line_number = data[min].line_number;
		data[min].first_number = tmp.first_number;
        data[min].second_number = tmp.second_number;
		data[min].line_number = tmp.line_number;
	}
}

long str_to_int(char * str)
{
	int i=0, exp = 0, pwr = 1;
	int tempExp = 0;
	long value = 0;
     
    //cout << "str: " << str << endl;

	while (str[i] != '\0')
	{
		i++;
	}

    exp = i-1;
    

	i =0 ;
	while (str[i] != '\0')
	{
        tempExp = exp;
        pwr = 1;
	    while (tempExp>=1)
		{
			pwr = pwr * 10;
			tempExp--;
		}

		switch (str[i])
        {
		case '0':
			 
			break;
		case '1':
			value += 1 * pwr;
			break;
		case '2':
			value += 2 * pwr;
			break;
		case '3':
			value += 3 * pwr;
			break;
		case '4':
			value += 4 * pwr;
			break;
		case '5':
			value += 5 * pwr;
			break;
		case '6':
			value += 6 * pwr;
			break;
		case '7':
			value += 7 * pwr;
			break;
		case '8':
			value += 8 * pwr;
			break;
		case '9':
			value += 9 * pwr;
			break;
		}
        exp--;
		i++;
	} // end while

	//cout << "value = " << value << endl;
	return value;
}
