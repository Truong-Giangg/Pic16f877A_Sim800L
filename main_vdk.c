//https://www.youtube.com/watch?v=wheLXTTLJqw
//https://microcontrollerslab.com/uart-interrupt-pic-microcontroller-mplab-xc8/
//https://www.hobbytronics.co.uk/mosfet-voltage-level-converter
//https://easyeda.com/alvaromautone/sim800l-and-pic-16f886
#define _XTAL_FREQ 20000000
#include <xc.h>
#include <string.h>
#include <stdio.h>
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit 
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits 
#pragma config CP = OFF         // Flash Program Memory Code Protection bit 
unsigned char *ptr, i, rev, rev1, rev2, rev3, rev4, buffer[90], xbuffer;
unsigned int new_sms;
void __interrupt() myISR(){     
    if(RCIF==1) {       //1 = The USART receive buffer is full
        if(RCSTAbits.OERR){       
            CREN = 0;   // Overrun error (can be cleared by clearing bit CREN)
            NOP();      // ASM delay
            CREN = 1;
        }
        rev = RCREG;    //USART Receive Data Register
        buffer[xbuffer++] = rev; 
        if(xbuffer>90) xbuffer = 0;
        if(rev == '+') rev1 = rev;           
        if(rev == 'C') rev2 = rev;
        if(rev == 'M') rev3 = rev;
        if(rev == 'T') rev4 = rev;
        if(rev1 == '+' && rev2 == 'C' && rev3 == 'M' && rev4 =='T') new_sms = 1;
        RCIF=0;         // interrupt flag
    }
}
char UART_Init(const long int baudrate){
  unsigned int x;
  x = (_XTAL_FREQ - baudrate*64)/(baudrate*64);   //SPBRG for Low Baud Rate
  if(x>255){                                      //If High Baud Rage Required
    x = (_XTAL_FREQ - baudrate*16)/(baudrate*16); //SPBRG for High Baud Rate
    BRGH = 1;                                     //Setting High Baud Rate
  }
  if(x<256){
    SPBRG = x;                                    //Writing SPBRG Register
    SYNC = 0;                                     //Setting Asynchronous Mode, ie UART
    SPEN = 1;                                     //Enables Serial Port
    TRISC7 = 1;                                   //As Prescribed in Datasheet
    TRISC6 = 1;                                   //As Prescribed in Datasheet
    CREN = 1;                                     //Enables Continuous Reception
    TXEN = 1;                                     //Enables Transmission
    return 1;                                     //Returns 1 to indicate Successful Completion
  }
  return 0;                                       //Returns 0 to indicate UART initialization failed
}
void UART_Write(char data){
  while(!TRMT);
  TXREG = data;
}
void UART_Write_Text(char *text){
  int i;
  for(i=0;text[i]!='\0';i++)
    UART_Write(text[i]);
}
void send_sms(char *n){
    UART_Write_Text("AT+CMGS=\"+84354778245\"\r\n");
    __delay_ms(500);
    UART_Write_Text(n);
    UART_Write_Text("\n\r");
    __delay_ms(500);
    //while(!TRMT);
    TXREG = 0x1A;       // '26' character(terminator character)
    __delay_ms(1000);
}
void ATinit(){
    UART_Write_Text("AT\r\n");          //ACK command
    __delay_ms(500);
    UART_Write_Text("ATE0\r\n");        //disable echo   
    __delay_ms(500);
    UART_Write_Text("AT&W\r\n");        // save this command
    __delay_ms(500);
    UART_Write_Text("AT+CSCS=\"GSM\"\r\n");
    __delay_ms(500);
    UART_Write_Text("AT+CMGF=1\r\n");   //format text
    __delay_ms(500);
    UART_Write_Text("AT+CNMI=1,2,0,0,0\r\n");
    __delay_ms(500);
    UART_Write_Text("AT+CSAS\r\n");
    __delay_ms(500);   
    send_sms("Khoi dong xong roi!!");   
    __delay_ms(500); 
}
void clear_buffer(){
    for(i =0; i<90; i++)    buffer[i] = ' ';
}
void delete_sms(){
    UART_Write_Text("AT+CMGDA=\"DEL ALL\"\r\n");
    __delay_ms(500);
}
void ADC_Init(){
  ADCON0 = 0b01000001; //ADC ON and FOSC/16 is selected
  ADCON1 = 0b11000000; // Internal reference voltage is selected, right justified
}
unsigned int ADC_Read(unsigned char channel){
  //Clearing the previous Channel Selection Bits 
  ADCON0 &= 0x11000101;         // bit 3,4,5 = 0, another bit not change
  ADCON0 |= channel<<3;         //Setting the required Bits 
  __delay_ms(2);                //Acquisition time to charge hold capacitor
  GO_nDONE = 1;                 //Initializes A/D Conversion
  while(GO_nDONE);              //Wait for A/D Conversion to complete
  return ((ADRESH<<8)+ADRESL);  //Returns Result    
}

void reverse(char* str, int len) {
    int i = 0, j = len - 1, temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
}
int intToStr(int x, char str[], int d) {
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    }  // If number of digits required is more, then add 0s at the beginning 
    while (i < d) 
        str[i++] = '0';  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
}
long int mypow(int x,int n) {
    int i; /* Variable used in loop counter */ 
    int number = 1;  
    for (i = 0; i < n; ++i) 
        number *= x; 
    return(number); 
} 
void ftos(float n, char* res, int afterpoint){     
    int ipart = (int)n;              // Extract integer part    
    float fpart = n - (float)ipart;  // Extract floating part  
    int i = intToStr(ipart, res, 0); // convert integer part to string   
    if (afterpoint != 0) {           // check for display option after point 
        res[i] = '.';                // add dot  
        // Get the value of fraction part upto given no. 
        // of points after dot. The third parameter  
        // is needed to handle cases like 233.007 
        fpart = fpart * mypow(10, afterpoint);  
        intToStr((int)fpart, res + i + 1, afterpoint); 
    } 
}
void main(void) {
    int adc, dem =0;
    TRISD = 0;                  // port D as output
    PORTD = 0;   
    TRISB = 0xff;               // port B as input(button)
    OPTION_REGbits.nRBPU = 0;   //PULL-UP PORT B
    GIE=1;      
    PEIE = 1;   
    PIE1bits.RCIE = 1;          //1 = Enables the USART receive interrupt 
    ADC_Init();
    UART_Init(9600);   
    ATinit();
    while(1){       
        adc = (ADC_Read(2));    // read light sensor
        if(adc < 400){                           
            PORTDbits.RD3 = 1;  // turn on yard light
        }
        else PORTDbits.RD3 = 0;
        adc = (ADC_Read(0));    // read LM35 sensor as AN0
        float vol = adc * 5.0/1024.0;   // 10mV=1 do, 0.4V*100=40 do 
        float tem = vol *100.0;
        adc = (ADC_Read(1));            // read gas sensor
        if(adc>700){
            PORTDbits.RD6 = 1;          // D6 as buzzer
            send_sms("canh bao khi ga ro ri");
            __delay_ms(1000);
        }
        else PORTDbits.RD6 = 0;
        
        if(PORTBbits.RB0 == 0){
            PORTDbits.RD1 = ~PORTDbits.RD1;     // bedroom
            while(!PORTBbits.RB0);   // debounce            
        }
        if(PORTBbits.RB1 == 0){
            PORTDbits.RD4 = ~PORTDbits.RD4;     // main hall
            while(!PORTBbits.RB1);   // debounce            
        }
        if(PORTBbits.RB2 == 0){
            PORTDbits.RD2 = ~PORTDbits.RD2;     // kitchen
            while(!PORTBbits.RB2);   // debounce            
        }
        if(PORTBbits.RB3 == 0){
            while(!PORTBbits.RB3);   // debounce 
        }
        if(new_sms){
            new_sms = 0;
            UART_Write_Text("AT+CMGR=1\r\n");   // read the first SMS
            __delay_ms(2000);
            ptr = strstr(buffer, "Ngu1");       // use pointer looking for string     
            if(strncmp(ptr, "Ngu1", 4)==0){     // compare string
                PORTDbits.RD1 = 1;              // when receive "Ngu1" turn on the light
                send_sms("da bat den phong ngu");
            }else{
                ptr = strstr(buffer, "Ngu0");
                if(strncmp(ptr, "Ngu0", 4)==0){      
                PORTDbits.RD1 = 0;
                send_sms("da tat den phong ngu");
                }
            }
            ptr = strstr(buffer, "Khach1");      
            if(strncmp(ptr, "Khach1", 4)==0){    
                PORTDbits.RD4 = 1;               
                send_sms("da bat den phong khach");
            }else{
                ptr = strstr(buffer, "Khach0");
                if(strncmp(ptr, "Khach0", 4)==0){      
                PORTDbits.RD4 = 0;
                send_sms("da tat den phong khach");
                }
            }
            ptr = strstr(buffer, "Bep1");          
            if(strncmp(ptr, "Bep1", 4)==0){      
                PORTDbits.RD2 = 1;                
                send_sms("da bat den phong bep");
            }else{
                ptr = strstr(buffer, "Bep0");
                if(strncmp(ptr, "Bep0", 4)==0){      
                PORTDbits.RD2 = 0;
                send_sms("da tat den phong bep");
                }
            }
            ptr = strstr(buffer, "info");         // check celsius
            if(strncmp(ptr, "info", 4)==0){       // compare string
                unsigned char a[20];               
                ftos(tem, a, 2);        // float to string
                //-----------------
                UART_Write_Text("AT+CMGS=\"+84354778245\"\r\n");
                __delay_ms(500);
                if(PORTDbits.RD2 == 1) UART_Write_Text("den phong bep dang bat\r\n");
                if(PORTDbits.RD2 == 0)  UART_Write_Text("den phong bep dang tat\r\n");
                if(PORTDbits.RD4 == 1) UART_Write_Text("den phong khach dang bat\r\n");
                if(PORTDbits.RD4 == 0)  UART_Write_Text("den phong khach dang tat\r\n");
                if(PORTDbits.RD1 == 1) UART_Write_Text("den phong ngu dang bat\r\n");
                if(PORTDbits.RD1 == 0)  UART_Write_Text("den phong ngu dang tat\r\n");
                if(PORTDbits.RD3 == 0)  UART_Write_Text("den ngoai san dang tat\r\n");
                if(PORTDbits.RD3 == 1)  UART_Write_Text("den ngoai san dang bat\r\n");
                UART_Write_Text("nhiet do phong la: ");
                UART_Write_Text(a);
                UART_Write_Text("\n\r");
                __delay_ms(500);
                while(!TRMT);
                TXREG = 0x1A;       // '26' character(terminator character)
                __delay_ms(1000);
                //------------------
            }
            clear_buffer();
            delete_sms();
            rev = 0;
            rev1 = 0;
            rev2 = 0;
            rev3 = 0;
            rev4 = 0;
        }   
    }
    
}
