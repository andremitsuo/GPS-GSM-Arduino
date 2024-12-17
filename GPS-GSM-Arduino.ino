// @andremitsuo
// 18/06/2012
// 
// This code will make the Arduino sending an SMS every time it finds a GPS coordinate. 
// coordinates can be programmed to be searched at any given time.
 
// # Product name: GPS/GPRS/GSM Module V2.0
// # Product SKU : TEL0051
// # Version     : 0.1

// # Steps:
// #        1. Turn the S1 switch to the EN(right side)
// #        2. Upload the sketch to the Arduino board
// #        3. Turn the S1 switch to the communication mode(left side)
// #        4. Turn the S2 switch to the Arduino side(right side)
// #        5. Turn the S3 switch to the ON side(right side)


String number = "????????"; // number of cellphone

String tmp;
double utc, lat, lon, alt;
char la_d, lo_d;

bool dadosOk = false, enviado = false;
byte gsmDriverPin[3] = {3,4,5};


void setup() 
{

}

void loop() 
{
  Serial.end();
  InitGpsMode();
   while(!dadosOk)
   {
     enviado = false;
     //Serial.println("GPS");
     gpsfuncao();    
     delay(100);     
   }   
   if(dadosOk){
     Serial.end();
     InitGsmMode();
     while(!enviado)
     {
       dadosOk = false;
       //Serial.println("GSM");
        gsmfuncao();        
        delay(100);       
     }
   }  
}

void InitGpsMode(){
  //ShutDown the GSM.
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(5, LOW);
  for(int i=0; i <= 7; i++){
    delay(1000); //wait 8 seconds to shutdown gsm connections
  }
  //Init the driver pins for GSM function
  for(int i = 0 ; i < 3; i++){
    pinMode(gsmDriverPin[i],OUTPUT);
  }
  Serial.begin(4800);
  digitalWrite(3,HIGH);//Disable the GSM mode
  digitalWrite(4,LOW);//Enable the GPS mode
}

void InitGsmMode(){
  //Init the driver pins for GSM function
  for(int i = 0 ; i < 3; i++){
    pinMode(gsmDriverPin[i],OUTPUT);
  }
  Serial.begin(115200);  
  digitalWrite(3,LOW);//Enable the GSM mode
  digitalWrite(4,HIGH);//Disable the GPS mode
  //Output GSM Timing
  digitalWrite(5,HIGH);
  delay(1500);
  digitalWrite(5,LOW);  
  for(int i=0; i <= 7; i++){
    delay(1000);
  }
}

void gsmfuncao()
{
  Serial.println("AT");
  for(int i=0; i <= 5; i++){
    delay(1000);    
  }
  Serial.println("AT+CMGF=1"); 
  for(int i=0; i <= 5; i++){
    delay(1000);    
  }
  Serial.println("AT+CMGS=\"" + number + "\"");
  for(int i=0; i <= 5; i++){
    delay(1000);    
  }  
  Serial.print("UTC:"); 
  Serial.println(utc, 0);
  Serial.print("Lat:");
  Serial.println(lat, 4);
  Serial.print("Lon:");
  Serial.println(lon, 4);
  Serial.print("Lon_D:");
  Serial.println(lo_d);
  Serial.print("Lat_D:");
  Serial.println(la_d);
  Serial.print("Alt:");
  Serial.println(alt, 2);
  delay(1000);
  Serial.println((char)26);
  delay(1000);  
  
  Serial.println();
  enviado = true;   
}

void gpsfuncao()
{  
  utc = UTC();  
  lat = latitude();  
  la_d = lat_dir();  
  lon = longitude();  
  lo_d = lon_dir();  
  alt = altitude();  
  dadosOk = true;
}

double Datatransfer(char *data_buf,char num)//convert the data to the float type
{                                           //*data_buf?the data array                                         //the number of the right of a decimal point
  double temp=0.0;
  unsigned char i,j;

  if(data_buf[0]=='-')//
  {
    i=1;
    //process the data array
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    //convert the int type to the float type
    for(j=0;j<num;j++)
      temp=temp/10;
    //convert to the negative number
    temp=0-temp;
  }
  else//for the positive number
  {
    i=0;
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    for(j=0;j<num;j++)
      temp=temp/10 ;
  }
  return temp;
}

char ID()//Match the ID commands
{ 
  char i=0;
  char value[6]={
    ','G','P','G','G','A'    };//match the gps protocol
  char val[6]={
    '0','0','0','0','0','0'    };

while(1)
  {
    if(Serial.available())
    {
      val[i] = Serial.read();//get the data from the serial interface
      if(val[i]==value[i])  //Match the protocol
      {   
        i++;
        if(i==6)
        {
          i=0;
          return 1;//break out after get the command
        }
      }
      else
        i=0;
    }
  }
}

void comma(char num)//get ','
{ 
  char val;
  char count=0;//count the number of ','

while(1)
  {
    if(Serial.available())
    {
      val = Serial.read();
      if(val==',')
        count++;
    }
    if(count==num)//if the command is right, run return
      return;
  }
}

double UTC()//get the UTC data -- the time
{
  char i;
  char time[9]={
    '0','0','0','0','0','0','0','0','0'
  };
  double t=0.0;
  String te;
  if(ID())//check ID
  {
    comma(1);//remove 1 ','
    //get the datas after headers
    while(1)
    {
      if(Serial.available())
      {
        time[i] = Serial.read();
        i++;
      }
      if(i==9)
      {
        i=0;
        t=Datatransfer(time,2);//convert data       
        if(t > 0 && t < 30000){
          t = t + 240000;
        }
        t=t-30000.00;//convert GMT-3 Time zone
        //Serial.println(t);//Print data
        return t;
      } 
    }
  }
}
double latitude()//get latitude
{
  double lt;
  char i;
  char lat[10]={
    '0','0','0','0','0','0','0','0','0','0'
  };

if( ID())
  {
    comma(2);
    while(1)
    {
      if(Serial.available())
      {
        lat[i] = Serial.read();
        i++;
      }
      if(i==10)
      {
        i=0;
        lt = Datatransfer(lat,4);//print latitude
        return lt;
      } 
    }
  }
}
char lat_dir()//get dimensions
{
  char i=0,val;

if( ID())
  {
    comma(3);
    while(1)
    {
      if(Serial.available())
      {
        val = Serial.read();
        //Serial.println(val);//print dimensions
        i++;
        return val;
      }
      if(i==1)
      {
        i=0;
        return val;
      } 
    }
  }
}
double longitude()//get longitude
{
  double ln;
  char i;
  char lon[11]={
    '0','0','0','0','0','0','0','0','0','0','0'
  };

if(ID())
  {
    comma(4);
    while(1)
    {
      if(Serial.available())
      {
        lon[i] = Serial.read();
        i++;
      }
      if(i==11)
      {
        i=0;
        ln = Datatransfer(lon,4);//print longitude
        return ln;
      } 
    }
  }
}

char lon_dir()//
{
  char i=0,val;

if(ID())
  {
    comma(5);
    while(1)
    {
      if(Serial.available())
      {
        val = Serial.read();
        //Serial.println(val);
        i++;
        return val;
      }
      if(i==1)
      {
        i=0;
        return val;
      }
    }
  }
}

double altitude()//get altitude data
{
  char i,flag=0;
  double at;
  char alt[8]={
    '0','0','0','0','0','0','0','0'
  };

if( ID())
  {
    comma(9);
    while(1)
    {
      if(Serial.available())
      {
        alt[i] = Serial.read();
        if(alt[i]==',')
          flag=1;
        else
          i++;
      }
      if(flag)
      {
        i=0;
        at = Datatransfer(alt,1);//print altitude data
        return at;
      } 
    }
  }
}
