//*******************************************
// AMPLIFICADOR V2.0  06/08/2022
//
//
//
//
//
//
//
//*******************************************


#include <16F877A.h>            //Define o modelo do microcontrolador
#device  adc=10
#include <stdio.h>
#include <stdlib.h>

 

#define lcd_enable pin_b2       // pino enable do LCD
#define lcd_rs     pin_b3       // pino RS  do LCD
#define alm        pin_e2       // pino alarme       
#define lcd_db4    pin_b4       // pino de dados d4 do LCD
#define lcd_db5    pin_b5       // pino de dados d5 do LCD
#define lcd_db6    pin_b6       // pino de dados d6 do LCD
#define lcd_db7    pin_b7       // pino de dados d7 do LCD
#define trisa 0b00010000        // 1 = entrada 0 = saida
#define trisb 0b00000111



 
// Fusíveis de configuração do microcontrolador
   
#FUSES NOWDT                    //Watch Dog Timer desabilitado
#FUSES HS                       //oscilador cristal HS
#FUSES NOPROTECT                //sem proteção para leitura da eprom 
#FUSES BROWNOUT                 //Resetar quando detectar brownout
#FUSES NOLVP                    //prog. baixa voltagem desabilitado
#FUSES NOCPD                    //Sem travar o chip

 
#use delay(clock=4000000)      //Define o cristal utilizado

#include <lcd.h>                //declaração da biblioteca do LCD
 

//variáveis
 
 float T1; 
 float T2;
 float TAMB;
 float BIAS0;
 float BIAS1;
 float BIAS2;
 float BIAS3;
 float P200V;
 float F6V3;
 float TRPOT;

 
 

 unsigned int16 vcanal0;
 unsigned int16 vcanal1;
 unsigned int16 vcanal2;
 unsigned int16 vcanal3;
 unsigned int16 vcanal4;

 unsigned int16 vcanal5_0;
 unsigned int16 vcanal5_1;
 unsigned int16 vcanal5_2;
 unsigned int16 vcanal5_3;
 unsigned int16 vcanal5_4;
 unsigned int16 vcanal5_5;
 unsigned int16 vcanal5_6;
 unsigned int16 vcanal5_7;

 unsigned int16 vcanal6;
 unsigned int16 vcanal7;
 unsigned int16 valor;
 float    Temperatura;

short  botao_c0 = 0; //  B0 
short  botao_c1 = 0; //  B1
short  botao_c2 = 0; //  B2  
short  botao_c3 = 0; //  B3

int controlseq = 0;

float BIAS0min=0.40;
float BIAS0max=1.30;
float BIAS1min=8.56;
float BIAS1max=14.34;
float BIAS2min=0.40;
float BIAS2max=1.30;
float BIAS3min=8.56;
float BIAS3max=14.34;
float P200Vmax= 280.0;
float P200Vmin=140.0;
float F6V3max =6.9;
float F6V3min =5.0;
float deltaT = 1;
unsigned int16 tempo_degrau = 3000; // 10s por degrau. atinge vmáx em 1,2 min.
unsigned int16 tempo_ativa_200V = 3000; //  30s antes de colocar VP
unsigned int16 per_int = 65535;
int vu_cal =10;

 

//Configuração dos conversores AD
  
config_conversor_AD()
{
setup_adc(ADC_CLOCK_DIV_32);
setup_vref(VREF_HIGH);
return(0);
}

tempo_atraso(unsigned int16 tatraso)
{
unsigned int16 conta_1ms;

for (conta_1ms =0; conta_1ms < tatraso; conta_1ms++)
{
delay_ms(1);
}

return(0);
}





// bargraph([VALOR DIGITAL],[LINHA 1/0],[ONDE COMEÇA A BARRA]);

bargraph(unsigned int16 barra,int linha,char offset)
{
char conta;
char contap;
char posicao;

if(barra>1023)
{
barra=1023;
}

posicao = barra >> 6;
for(conta=offset;conta<=posicao+1;conta++)
{
	caracter_inicio(linha,conta);
	envia_byte_lcd(1,0xFF);
}

for(contap=conta;contap<=16;++contap)
{ 
	caracter_inicio(linha,contap);
	envia_byte_lcd(1,0x20);
}
return(0);
}


// alarme_temperatura sensor T1V,T2V,TAMBV,TRPOTV;

float temperatura_sensores(float valor)
{

Temperatura =  valor*0.489;   //Temperatura =  (valor/1023)*(5/0.01);
 return(Temperatura);
} 

le_botoes()
{
short acionado=0;

 botao_c0 =  input(pin_c0); //  B0
 botao_c1 =  input(pin_c1); //  B1
 botao_c2 =  input(pin_c2); //  B2  
 botao_c3 =  input(pin_c3); //  B3

 acionado = botao_c0||botao_c1||botao_c2||botao_c3 ;

 return(acionado); 
}


testa_tecla()
{
 short testeok =0;
 botao_c0 = 0;
 botao_c1 = 0;
 botao_c2 = 0; 
 botao_c3 = 0;

limpa_lcd();
caracter_inicio(1,1);
printf(escreve_lcd,"Tst botoes.Sair:");
caracter_inicio(2,1);
printf(escreve_lcd,"4 botoes juntos ");
delay_ms(3000);
limpa_lcd();
caracter_inicio(1,1);
printf(escreve_lcd,"C0 C1 C2 C3");


while(testeok == 0)
{  
le_botoes();
caracter_inicio(2,1);
printf(escreve_lcd,"%u",botao_c0);
caracter_inicio(2,4);
printf(escreve_lcd,"%u",botao_c1);
caracter_inicio(2,7);
printf(escreve_lcd,"%u",botao_c2);
caracter_inicio(2,10);
printf(escreve_lcd,"%u",botao_c3);

testeok=botao_c0 & botao_c1 &	botao_c2 & botao_c3;
}

return(0);

}		







Mostra_variaveis_sistema_bias()
{

                //Mostra variaveis tensão
               	limpa_lcd(); 
				caracter_inicio(1,1);
				printf(escreve_lcd,"B0:%3.1f",BIAS0);
				printf(escreve_lcd,"   ");
				printf(escreve_lcd,"B1:%3.1f",BIAS1);
				caracter_inicio(2,1);
				printf(escreve_lcd,"B2:%3.1f",BIAS2);
				printf(escreve_lcd,"   ");
				printf(escreve_lcd,"B3:%3.1f",BIAS3);
                

return(0);
}

Mostra_variaveis_sistema_Vp_Vh()
{
                limpa_lcd();
 			    caracter_inicio(1,1);
				printf(escreve_lcd,"VP:%3.1f",P200V);
				printf(escreve_lcd," ");
				printf(escreve_lcd,"VF:%3.1f",F6V3); 
             
return(0);
}





Mostra_variaveis_termicas()
{
limpa_lcd();
caracter_inicio(1,1);
printf(escreve_lcd,"T1:%3.0f",T1);
caracter_inicio(1,11);
printf(escreve_lcd,"T2:%3.0f",T2);
caracter_inicio(2,1);
printf(escreve_lcd,"Tamb:%3.0f",TAMB);
return(0);
}






leitura_canais_ad_dig()
{

// leitura_canais_ad_dig() 
// Le os canais AN0 - AN7 variavel 0-1023 vcanalx  0=<x=<7
// le o estensor de portas analogicas 4051
//  vcanal0;  T1
//  vcanal1;  T2
//  vcanal2;  TAMB
//  vcanal3;  TESTE FTE A
//  vcanal4;  TESTE FTE B
//  vcanal5_0; BIAS 0
//  vcanal5_1; BIAS 1
//  vcanal5_2; BIAS 2
//  vcanal5_3; BIAS 3
//  vcanal5_4; +200V antes RELE
//  vcanal5_5; +6,3V antes RELE
//  vcanal5_6; TRPOT
//  vcanal5_7; livre
//  vcanal6; VUA - esquerdo
//  vcanal7; VUB - direito

set_adc_channel(0); // T1
delay_us(50);
vcanal0 = read_adc();
delay_us(50);


set_adc_channel(1); // T2
delay_us(50);
vcanal1 = read_adc();
delay_us(50);

set_adc_channel(2); // TAMB
delay_us(50);
vcanal2 = read_adc();
delay_us(50);

set_adc_channel(3);  // Mapeado teste FTE A esq
delay_us(50);
vcanal3 = read_adc();
delay_us(50);
 
set_adc_channel(4);  // Mapeado teste FTE B dir
delay_us(50);
vcanal4 = read_adc();  
delay_us(50);


// le o estensor de portas analogicas 4051 

set_adc_channel(5);

delay_us(500);
output_bit(pin_d5,0); // a
output_bit(pin_d6,0); // b
output_bit(pin_d7,0); // c
delay_us(500);
vcanal5_0 = read_adc();
delay_us(50);


output_bit(pin_d5,1); // a
output_bit(pin_d6,0); // b
output_bit(pin_d7,0); // c
delay_us(500);
vcanal5_1 = read_adc();
delay_us(50);

;
output_bit(pin_d5,0); // a
output_bit(pin_d6,1); // b
output_bit(pin_d7,0); // c
delay_us(500);
vcanal5_2 = read_adc();
delay_us(50);


output_bit(pin_d5,1); // a
output_bit(pin_d6,1); // b
output_bit(pin_d7,0); // c
delay_us(500);
vcanal5_3 = read_adc();
delay_us(50);


output_bit(pin_d5,0); // a
output_bit(pin_d6,0); // b
output_bit(pin_d7,1); // c
delay_us(500);
vcanal5_4 = read_adc();
delay_us(50);

;
output_bit(pin_d5,1); // a
output_bit(pin_d6,0); // b
output_bit(pin_d7,1); // c
delay_us(500);
vcanal5_5 = read_adc();
delay_us(50);


output_bit(pin_d5,0); // a
output_bit(pin_d6,1); // b
output_bit(pin_d7,1); // c
delay_us(500);
vcanal5_6 = read_adc();
delay_us(50);


output_bit(pin_d5,1); // a
output_bit(pin_d6,1); // b
output_bit(pin_d7,1); // c
delay_us(500);
vcanal5_7 = read_adc();
delay_us(50);


set_adc_channel(6);  // VU canal A esq
delay_us(50);
vcanal6 = read_adc();
delay_us(50);

set_adc_channel(7);  // VU canal B dir
delay_us(50);
vcanal7 = read_adc();
delay_us(50);



 T1    = temperatura_sensores(vcanal0);
 T2    = temperatura_sensores(vcanal1);
 TAMB  = temperatura_sensores(vcanal2);
 BIAS0 = vcanal5_0*0.049;   // fator para 5V 0.0049
 BIAS1 = vcanal5_1*0.049;    // fator para 5V x 10
 BIAS2 = vcanal5_2*0.049;
 BIAS3 = vcanal5_3*0.049;    // fator para 5V x 10
 P200V = vcanal5_4*0.49;     // fator para 5V x 100
 F6V3  = vcanal5_5*0.049;    // fator para 5V x 10
 TRPOT = vcanal5_6*0.0049;



return(0);
}









controla_filamento(int sel)
{

//controla_filamento_ST(1-7)
// para filamento = 0 usar:  liga_desliga_6V3()



switch (sel) 
{
 	case 0:	output_bit(pin_c5,0); // a
			output_bit(pin_c6,0); // b
			output_bit(pin_c7,0); // c
		    break;

	case 1:	output_bit(pin_c5,1); // a
			output_bit(pin_c6,0); // b
			output_bit(pin_c7,0); // c
			break;

 	case 2:	output_bit(pin_c5,0); // a
			output_bit(pin_c6,1); // b
			output_bit(pin_c7,0); // c
			break;

	case 3:	output_bit(pin_c5,1); // a
			output_bit(pin_c6,1); // b
			output_bit(pin_c7,0); // c
			break;

	case 4:	output_bit(pin_c5,0); // a
			output_bit(pin_c6,0); // b
			output_bit(pin_c7,1); // c
			break;

	case 5:	output_bit(pin_c5,1); // a
			output_bit(pin_c6,0); // b
			output_bit(pin_c7,1); // c
			break;

	case 6:	output_bit(pin_c5,0); // a
			output_bit(pin_c6,1); // b
			output_bit(pin_c7,1); // c
			break;

	case 7:	output_bit(pin_c5,1); // a
			output_bit(pin_c6,1); // b
			output_bit(pin_c7,1); // c
 	    	break;

}


return(0);  
}


Beep_Erro(int beeps)
{
int nbeeps;

For (nbeeps=0; nbeeps < beeps; nbeeps++)
{
output_bit(pin_d4,1);
delay_ms(90);
output_bit(pin_d4,0);
delay_ms(90);
}

return(0);
}









testa_FTE_A_B() 
{ 

short erroA =1;
short erroB =1;


limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"Teste AF,s e TRN");
output_bit(pin_c4,1); // fte teste ativo
tempo_atraso(3000);


while(erroA == 1 || erroB == 1 )
{


if(vcanal3 < 920)
{
erroA = 0;
}

if(vcanal3 < 920)
{
erroB = 0;
}


if(vcanal3 > 920)
{
beep_erro(1);
erroA = 1;
caracter_inicio(2,1);
printf(escreve_lcd,"Falha Canal (A) ");
tempo_atraso(3000);
} 



if(vcanal4 > 920)
{
beep_erro(1);
erroB = 1;
caracter_inicio(2,1);
printf(escreve_lcd,"Falha Canal (B) ");
tempo_atraso(3000);
}

}


if (erroA == 0 && erroB == 0)
{
caracter_inicio(2,1);
printf(escreve_lcd,"Teste sem Falhas");
}
output_bit(pin_c4,0); // fte teste desativado
delay_ms(3000);

return(0);
}

liga_led_(short vermelho,short verde)
{
output_bit(pin_d0,vermelho);
output_bit(pin_d1,verde);
return(0);
}





liga_desliga_200V(short chave)
{
output_bit(pin_b0,chave); // +200V
return(0);
}

liga_desliga_6V3(short chavef)
{
output_bit(pin_b1,chavef); // +6,3 V
return(0);
}


partida_filamento(unsigned int16 tempo_passo,int iniciopf,int fimpf)
{
unsigned int16 partida;
unsigned int16 tpasso;

limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"Aumentando Vf(V)");
delay_ms(3000);

for (partida=iniciopf; partida <= fimpf; partida++)
{ 
controla_filamento(partida);
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"T1:%3.0f",T1);
caracter_inicio(1,11);
printf(escreve_lcd,"T2:%3.0f",T2);
caracter_inicio(2,1);
printf(escreve_lcd,"Vf(V):%f",F6V3);
printf(escreve_lcd,"  D:%lu",partida);




for(tpasso=0; tpasso < tempo_passo; tpasso++)
{
delay_ms(1);
}




}
return(0);
}

desliga_filamento(unsigned int16 tempo_passo_d,int iniciodf,int fimdf)
{
unsigned int16 desliga;
unsigned int16 tpassod;

limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"Reduzindo Vf(V)");
delay_ms(3000);

for (desliga=iniciodf; desliga > fimdf; desliga--)
{ 

controla_filamento(desliga);
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"T1:%3.0f",T1);
caracter_inicio(1,11);
printf(escreve_lcd,"T2:%3.0f",T2);
caracter_inicio(2,1);
printf(escreve_lcd,"VH(V):%f",F6V3);
printf(escreve_lcd," D:%lu",desliga);

for(tpassod=0; tpassod < tempo_passo_d; tpassod++)
{
delay_ms(1);
}

;
}

return(0);
} 

alarme_fontes()

{

if( P200V > P200Vmax || P200V <  P200Vmin || F6V3 > F6V3max || F6V3 < F6V3min)
{
limpa_lcd();  
caracter_inicio(1,1);
printf(escreve_lcd,"Falha na Fonte");
caracter_inicio(2,1);
printf(escreve_lcd,"VP:%3.1f",P200V);
printf(escreve_lcd,"Vf:%3.1f",F6V3);
Beep_Erro(5);
liga_desliga_200V(0);
liga_desliga_6V3(0);
delay_ms(5000);
caracter_inicio(1,1);
printf(escreve_lcd,"Desl.Chave Geral");
while(1)
{
}
}

limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"VP e Vf Normais ");
caracter_inicio(2,1);
printf(escreve_lcd,"Vf=%1.1f",F6V3);
printf(escreve_lcd," ");
printf(escreve_lcd,"VP=%3.1f",P200V);
delay_ms(5000);


return(0);
}













alarme_polarizacoes()
{

short errobias =0;

limpa_lcd(); 


if(BIAS0 > BIAS0max ||  BIAS0 < BIAS0min || BIAS1 > BIAS1max || BIAS1 < BIAS1min )
{
caracter_inicio(1,1);
printf(escreve_lcd,"Canal A Falha.");
Beep_Erro(5);
delay_ms(3000);
errobias =1;


}

if(BIAS2 > BIAS2max || BIAS2 < BIAS2min || BIAS3 > BIAS3max || BIAS3 < BIAS3min)
{
caracter_inicio(2,1);
printf(escreve_lcd,"Canal B Falha.");
Beep_Erro(5);
delay_ms(3000);
errobias =1;

}

if (errobias==1)
{
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"Desligue o AMP");
caracter_inicio(2,1);
printf(escreve_lcd,"C2-> Desligar ");
delay_ms(8000);
}

return(errobias);
}




alarme_temperatura_ligar()
{


limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"Teste Termico ");
caracter_inicio(2,1);
printf(escreve_lcd,"Emerg. C3:Sair");
   
tempo_atraso(3000);  
limpa_lcd(); 

while ((T2-TAMB > DeltaT || T1-TAMB > DeltaT) &&  botao_c3 == 0)
{
caracter_inicio(1,1);
printf(escreve_lcd,"Protecao Ativada!"); 
caracter_inicio(2,1);
printf(escreve_lcd,"T1:%3.0f",T1);
caracter_inicio(2,11);
printf(escreve_lcd,"T2:%3.0f",T2);

le_botoes();

}

return(0);
}













						// a cada periodo o timer chama esta interrupção
#INT_TIMER1  
void timer1_isr(void)
{
leitura_canais_ad_dig();
 clear_interrupt(INT_TIMER1);       // Clear  interrupt flag bit
 }


						// ativa interrupção interna  timer_1
Ativa_timer_1(unsigned int16 tempo)
{ 

setup_timer_1(T1_INTERNAL | T1_DIV_BY_1);
set_timer1(tempo);   
clear_interrupt(INT_TIMER1);
enable_interrupts(INT_TIMER1); 
enable_interrupts(GLOBAL); 
return(0);
}

// desativa interrupção interna  timer_1

Desativa_timer_1()
{
disable_interrupts(INT_TIMER1); 
disable_interrupts(GLOBAL); 
return(0);
}





//Programa principal 


void main() 
{
// ligar chave ON OFF
// inicializa sistema
// 3 beeps = erro FTE 
// 6 beeps = Fonte

config_conversor_AD();         								 //configura inicializa A/D
Ativa_timer_1(per_int);          								 // ATIVA interrupção de coleta A/D
inicializa_lcd();               								//Inicializa o LCD
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd," Valvulado ECL82 ");    
caracter_inicio(2,1);
printf(escreve_lcd,"ACG 2022 V.82022 ");
delay_ms(6000);
caracter_inicio(2,1);
printf(escreve_lcd,"Testando Fontes  ");
delay_ms(6000);
liga_desliga_200V(0);          								 // desativa preventivamente Tensoes +200V e 6,3V OFF AMP
liga_desliga_6V3(0);
controla_filamento(7);   // ativa 6.3V
delay_ms(3000);
alarme_fontes();			   						 // testa fontes
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd," Valvulado ECL82 ");    
caracter_inicio(2,1);
printf(escreve_lcd,"Teste FTE/Temp");
delay_ms(3000);
alarme_temperatura_ligar();
testa_FTE_A_B();
                                  									//  testa_tecla();                                
										//  Se desejar o teste de teclas na inicialização, ative-o removendo do modo comentário
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd," Valvulado ECL82 ");    
caracter_inicio(2,1);

printf(escreve_lcd,"Teste Finalizado");
delay_ms(3000);
caracter_inicio(2,1);
printf(escreve_lcd,"Modo de Espera ");









while(1)
{

//while(le_botoes() == 0)


if(botao_C0 == 1 && controlseq == 0)
{

						// Entra em modo de espera
liga_led_(1,0);                				  // vermelho   se implementado no circuito.
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"Valvulado ECL82 ");  
caracter_inicio(2,1);
printf(escreve_lcd,"Modo de Espera  ");
delay_ms(3000);	  		
controla_filamento(0);        				// Potencia minima filamentos
liga_desliga_6V3(1);          				// Liga Filamento
partida_filamento(tempo_degrau,0,3); 			// liga e vai para espera.
limpa_lcd(); 
caracter_inicio(1,1);
printf(escreve_lcd,"ECL82 Em Espera "); 
caracter_inicio(2,1);
 printf(escreve_lcd,"C0:Oper.C2:Desl.");
controlseq =1;
}
 
while(le_botoes() == 0)
{
}

if (botao_C0 == 1 && controlseq == 1)
{
               					 					//Entra em modo operação saindo da espera
	controla_filamento(3);        							// Potencia espera filamentos
	 liga_desliga_6V3(1);         							// Liga Filamento
                  partida_filamento(tempo_degrau,3,7); 						// liga e vai para espera.
                  limpa_lcd(); 
	 caracter_inicio(1,1);
                  printf(escreve_lcd,"Ativando AT nas ");
                  caracter_inicio(2,1);
                  printf(escreve_lcd,"Valvulas.Aguarde");
	 delay_ms(tempo_ativa_200V);
                  liga_desliga_200V(1);  
                  delay_ms(4500);
                  alarme_polarizacoes(); 
                  limpa_lcd(); 
	 caracter_inicio(1,1);
                  printf(escreve_lcd,"ECL82  Operando ");
                  caracter_inicio(2,1);
                  printf(escreve_lcd,"C0:Esp. C2:Desl.");
                  liga_led_(0,1);  // verde
	delay_ms(5000);
                  controlseq = 2;
}
 
while(le_botoes() == 0 && botao_c0 == 0 && controlseq == 2)
{


bargraph(vcanal6*vu_cal,1,0);
bargraph(vcanal7*vu_cal,2,0);

if(le_botoes()== 1 && botao_c1 == 1)
{

while(le_botoes()==1)
{
}

while (le_botoes()== 0 && botao_c1 == 0)
{
Mostra_variaveis_sistema_bias();
delay_ms(500);
}

while(le_botoes()==1)
{
}

while (le_botoes()== 0 && botao_c1 == 0)
{
Mostra_variaveis_sistema_Vp_Vh();
delay_ms(500);
}

while(le_botoes()==1)
{
}

while(le_botoes()== 0 && botao_c1 == 0)
{
Mostra_variaveis_termicas();
delay_ms(500);
}

}
}


if (botao_c0 == 1 && controlseq == 2 )
{
               							 //Entra em modo espera saindo da operação
				  limpa_lcd(); 
				  caracter_inicio(1,1);
                  printf(escreve_lcd,"ECL82 Retornando");
                  caracter_inicio(2,1);
                  printf(escreve_lcd,"Para Modo Espera");
				  delay_ms(3000);
                  liga_desliga_200V(0);
                  desliga_filamento(tempo_degrau,7,3); // liga e vai para espera.
                  limpa_lcd(); 
				  caracter_inicio(1,1);
                  printf(escreve_lcd,"ECL82 Em Espera ");
                  caracter_inicio(2,1);
                  printf(escreve_lcd,"C0:Oper.C2:Desl.");
				  controlseq = 1;
}


if (botao_c2 == 1 && controlseq == 2 || botao_c2 == 1 && controlseq == 1 )
{
               							 //Entra em modo DESLIGAR saindo da operação
				  
                  limpa_lcd();   
				  caracter_inicio(1,1);
                  printf(escreve_lcd,"ECL82 Desligando");
                  liga_desliga_200V(0);
                  delay_ms(3000);
              	  liga_desliga_6V3(0);
                  delay_ms(7000);
				  limpa_lcd();   
				  caracter_inicio(1,1);
                  printf(escreve_lcd,"ECL82 desligado ");
                  caracter_inicio(2,1);
                  printf(escreve_lcd,"DESL.CHAVE GERAL");
				  Beep_Erro(2);
				  
				while(1)
               {
               } 

}
 

}


}


