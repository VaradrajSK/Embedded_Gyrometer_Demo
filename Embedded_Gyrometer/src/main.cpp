#include "stdio.h"
#include "mbed.h"
#include "Arial28x28.h"
#include "SPI_TFT_ILI9341.h"
#include "string"

// Gyro spi pins
SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_1);

// the TFT is connected to SPI pin 11-14
SPI_TFT_ILI9341 TFT(PF_9, PF_8, PF_7, PC_2, PD_12, PD_13,"TFT"); // mosi, miso, sclk, cs, reset, dc

// x,y,z rotation values from gyro
int16_t final_x, final_y, final_z=0;

// flag to display on tft
uint8_t d_flag=1;

// flag to display on read_gyro data
uint8_t r_flag=0;

// debug variables
int i,j,k=0;

// Variables that contain x,y,z rotation in radians
float datax;
float datay;
float dataz;

// number of steps
int step=0;

// distance using s=rw
float dist1=0;

// distance using constant step size
float dist2=0;

void gyro_init(); //Function to initialize L3GD20 gyro
void gyro_read(); //Function to read L3GD20 gyro data and compute the distance
void disp();      //Function to set flag to display data on the TFT
void g_read();    //Function to read L3GD20 gyro data and compute the distance
void step_inc();  //Function to increment steps based on gyro reading
void tft_init();  //Function to initialize TFT
void tft_disp();  // Function to display data on TFT


int main()
{
  // tft display tikcer
  Ticker t1;
  t1.attach(&disp,1000ms);

  // gyro read ticker
  Ticker t2;
  t2.attach(&g_read,80ms);

  // step increment ticker
  Ticker t3;
  t3.attach(&step_inc,300ms);

  gyro_init();
  tft_init();
  
  while(1){
    if(r_flag==1)
    {
      gyro_read();
      r_flag=0;
    }

    if(d_flag==1)
    {
      tft_disp();
      d_flag=0;
    }
  }
}

/* 
  Function to initialize L3GD20 gyro
*/
void gyro_init()
{
  // Chip must be deselected
  cs = 1;
  // Setup the spi for 8 bit data, high steady state clock,
  // second edge capture, with a 1MHz clock rate
  spi.format(8,3);
  spi.frequency(1000000);


////////////Who am I????
  cs = 0;
  // wait_us(200);
  // Send 0x8f, the command to read the WHOAMI register
  spi.write(0x8F);
  // Send a dummy byte to receive the contents of the WHOAMI register
  int whoami = spi.write(0x00);
  printf("WHOAMI register = 0x%X\n", whoami);

////////////Control Register 1
  //Read Control1 Register
  spi.write(0xA0);
  int ctrl1 = spi.write(0x00);
  printf("Control1 = 0x%X\n", ctrl1);

  cs = 1;
  cs = 0;      
  spi.write(0x20);
  spi.write(0x0F);
  cs = 1;
  
  cs = 0;
  //Read Control1 Register
  ctrl1 = spi.write(0x00);
  printf("control1 = 0x%X\n", ctrl1);
  cs = 1;

////////////Control Register 4
  //Read Control4 Register
  cs = 0;      
  spi.write(0x24);
  spi.write(0x12);
  cs = 1;
  cs = 0;      
  //Read Control4 Register
  spi.write(0xA4);

  // Send a dummy byte to receive the contents of the Control 4 register
  int ctrl4 = spi.write(0x00);
  printf("control4 = 0x%X\n", ctrl4);
  cs = 1;



////////////Control Register 3
  cs = 0;      
  //Read Control3 Register
  spi.write(0xA3);

  // Send a dummy byte to receive the contents of the Control 3 register
  int ctrl3 = spi.write(0x00);
  printf("control3 = 0x%X\n", ctrl3);
  cs = 1;

  cs = 0;      
  spi.write(0x23);
  spi.write(0x20);
  cs = 1;
  // Send a dummy byte to receive the contents of the Control 2 register
  ctrl3 = spi.write(0x00);
  printf("control3 = 0x%X\n", ctrl3);
  cs = 1;
  
}

/* 
  Function to read L3GD20 gyro data and compute the distance
*/
void gyro_read()
{
  cs = 0;
  spi.write(0xE8);  
  int OUT_X_L = spi.write(0x00);
  int OUT_X_H = spi.write(0x00);
  int OUT_Y_L = spi.write(0x00);
  int OUT_Y_H = spi.write(0x00);
  int OUT_Z_L = spi.write(0x00);
  int OUT_Z_H = spi.write(0x00);
  cs = 1;

  int16_t final_x = (OUT_X_H<<8)|(OUT_X_L);
  int16_t final_y = (OUT_Y_H<<8)|(OUT_Y_L);
  int16_t final_z = (OUT_Z_H<<8)|(OUT_Z_L);      

  // rps = final_x * dps * (degrees_to_radians)
  // 0.000610f = 0.03500f * 0.017453292519943295f; 
  datax = final_x * 0.000610f; 
  // datay = final_y * 0.000610f; 
  // dataz = final_z * 0.000610f; 

  // thresholds set for 0.15, 0.2 and 0.25
  // these values have been chosen based on the data acquired during test runs and 
  // derived deom the graphs when steps are taken
  if(datax>0.15 || datax<-0.15)
  { 
    if(datax <=0)
      datax = -datax;
      
    if(datax>0.25 || datax<-0.25)
    {
      dist1 = dist1 + datax*120;
      dist2+=40;
      k++;
    }
    else if(datax>0.20 || datax<-0.20)
    {
      dist1 = dist1 + datax*100;  
      dist2+=30;
      j++;
    }      
    else
    {
      dist1 = dist1 + datax*90;
      dist2+=20;
      i++;
    }
  }
}

/* 
  Function to set flag to display data on the TFT
  refresh rate = 1 sec
*/
void disp()
{ 
  if(d_flag==0)
  {
    d_flag=1;
  }
}

/* 
  Function to read L3GD20 gyro data and compute the distance
  sampling rate = 80ms
*/
void g_read()
{ 
  if(r_flag==0)
  {
    r_flag=1;
  }
}

/* 
  Function to increment steps based on gyro reading 
  sampling rate = 500ms
*/
void step_inc()
{ 
  if(datax>0.15 || datax<-0.15)
  { 
    step++;
  }
}

/* 
  Function to initialize TFT
*/
void tft_init()
{
  
  TFT.claim(stdout);      // send stdout to the TFT display

  TFT.background(Black);    // set background to black
  TFT.foreground(White);    // set chars to white
  TFT.cls();                // clear the screen

  TFT.background(Black);
  TFT.cls();

  TFT.cls();
  TFT.rect(0,0,400,400,LightGrey);

  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,10);
  TFT.printf("ECE-GY 6483");

  TFT.foreground(Red);
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,50);
  TFT.printf("Embedded");  

  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,90);
  TFT.printf("Gyrometer");

  TFT.foreground(Blue);
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,220);
  TFT.printf("Parishi");
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,260);
  TFT.printf("VaradrajK");

  wait_us(2000000); 

  TFT.cls();
  TFT.rect(0,0,400,400,LightGrey);
  TFT.foreground(White);    // set chars to white
}

/* 
  Function to display data on TFT
*/
void tft_disp()
{
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,10);
  TFT.printf("            ");

  TFT.foreground(Red);
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,10);
  TFT.printf("Using rw");

  TFT.foreground(White); 
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,50);
  TFT.printf("Dist = %0.2fm",dist1/100);
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,90);
  TFT.printf("Cal = %0.2f",(i+j+k)*0.04/6);

  TFT.foreground(Red);
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,180);
  TFT.printf("Using step");
  TFT.foreground(White); 
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,220);
  TFT.printf("Dist = %0.2fm",dist2/100);
  TFT.set_font((unsigned char*) Arial28x28);
  TFT.locate(15,260);
  TFT.printf("Steps = %d",step);
}
