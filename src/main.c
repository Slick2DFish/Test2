#include <stm32f10x.h>
#include "stm32f10x_conf.h"


void initUART()
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    GPIOA->CRL |= GPIO_CRL_CNF2_1 | GPIO_CRL_MODE2; //TX -> Output 50 MHz, Push-Pull
    GPIOA->CRL &= ~(GPIO_CRL_CNF2_0);

    USART2->CR1 |= USART_CR1_RE |USART_CR1_TE | USART_CR1_UE; //RX an, TX an, UART an

    USART2->BRR = 0xEA6; //9600 Baudrate
}

void sendChar(char c)
{
     if(USART2->SR & USART_SR_TXE)
        {

            USART2->DR = c;
             while(!(USART2->SR & USART_SR_TC));
        }
}

void sendString(char * s)
{
    while(*s != 0)
    {
        if(USART2->SR & USART_SR_TXE)
        {
        USART2->DR = *s;
        s++;
        }
    }
}


void sendNumber(uint16_t val)
{
uint8_t i;
char s[10];

if(val == 0)
{
    if(USART2->SR & USART_SR_TXE)
        {
             char c = '0';
             USART2->DR = c;
             while(!(USART2->SR & USART_SR_TC));

             return;
        }
}

for(i = 0; i < 5; i++)
{
    if(val > 0)
    {
        s[4-i] = 48+ (val % 10);
        val /= 10;
    }else{
        s[4-i] = 0x00;
    }
}

for(i = 0; i< 10; i++)
{
    if(s[i] != 0x00)
    {
      if(USART2->SR & USART_SR_TXE)
        {

             USART2->DR = s[i];
             while(!(USART2->SR & USART_SR_TC));


        }
    }
}
}

void send8BitInt(uint8_t i)
{
     if(USART2->SR & USART_SR_TXE)
        {

            USART2->DR = (uint8_t)i;
             while(!(USART2->SR & USART_SR_TC));
        }
}

void initADC()
{
     RCC->APB2ENR |= (1<<2);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);

    GPIO_InitTypeDef GPIO_Struct;
    ADC_InitTypeDef ADC_Struct;

    GPIO_Struct.GPIO_Pin = GPIO_Pin_0;
    GPIO_Struct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_Struct);

    GPIOA->CRH |= (1<<8) | (1<<9); //Output Mode, 50MHz
    GPIOA->CRH &= ~((1<<10) | (1<<11)); //General Purpose Output

    ADC_Struct.ADC_Mode = ADC_Mode_Independent;
    ADC_Struct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_Struct.ADC_NbrOfChannel = 1;
    ADC_Struct.ADC_ScanConvMode = ENABLE;
    ADC_Struct.ADC_ContinuousConvMode = ENABLE;
    ADC_Struct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_Init(ADC1, &ADC_Struct);


   ADC_RegularChannelConfig(ADC1, 0, 1, ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);


}

uint16_t val = 0;

int main(void)
{
    initUART();
    initADC();


    while(1)
    {


        if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
           val = ADC_GetConversionValue(ADC1);


        uint8_t volume = ((uint8_t) (val / 16));
        send8BitInt(255); // 255 = Flag for value
        send8BitInt(volume); //Send Volume



        GPIOA->BSRR |= (1<<10); //Pin auf HIGH
        for(uint32_t i = 0; i < 1000; i++){ for(uint32_t i = 0; i < 10; i++);}
        GPIOA->BSRR |= (1<<26); //Pin auf LOW
        for(uint32_t i = 0; i < val; i++){ for(uint32_t i = 0; i < 10; i++);}

    }
}





//st-flash write /home/maciel/eclipse-workspace/Test2/Debug/Test2.hex 0x8000000

//arm-none-eabi-objcopy -O binary /home/maciel/eclipse-workspace/Test2/Debug/Test2.elf /home/maciel//eclipse-workspace/Test2/Debug/Test2.bin


