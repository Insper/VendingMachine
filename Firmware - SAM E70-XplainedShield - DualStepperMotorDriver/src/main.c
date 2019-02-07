/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define M1_A1			PIOA
#define M1_A1_ID        ID_PIOA
#define M1_A1_IDX       5
#define M1_A1_IDX_MASK  (1 << M1_A1_IDX)

#define M1_A2           PIOA
#define M1_A2_ID        ID_PIOA
#define M1_A2_IDX       6
#define M1_A2_IDX_MASK  (1 << M1_A2_IDX)

#define M1_B1           PIOD
#define M1_B1_ID        ID_PIOD
#define M1_B1_IDX       27
#define M1_B1_IDX_MASK  (1 << M1_B1_IDX)

#define M1_B2           PIOD
#define M1_B2_ID        ID_PIOD
#define M1_B2_IDX       11
#define M1_B2_IDX_MASK  (1 << M1_B2_IDX)

#define M1_ENA           PIOC
#define M1_ENA_ID        ID_PIOC
#define M1_ENA_IDX       19
#define M1_ENA_IDX_MASK  (1 << M1_ENA_IDX)

#define M1_ENB           PIOA
#define M1_ENB_ID        ID_PIOA
#define M1_ENB_IDX       2
#define M1_ENB_IDX_MASK  (1 << M1_ENB_IDX)





#define M2_A1			PIOA
#define M2_A1_ID        ID_PIOA
#define M2_A1_IDX       17
#define M2_A1_IDX_MASK  (1 << M2_A1_IDX)

#define M2_A2           PIOC
#define M2_A2_ID        ID_PIOC
#define M2_A2_IDX       9
#define M2_A2_IDX_MASK  (1 << M2_A2_IDX)

#define M2_B1           PIOD
#define M2_B1_ID        ID_PIOD
#define M2_B1_IDX       25
#define M2_B1_IDX_MASK  (1 << M2_B1_IDX)

#define M2_B2           PIOD
#define M2_B2_ID        ID_PIOD
#define M2_B2_IDX       21
#define M2_B2_IDX_MASK  (1 << M2_B2_IDX)

#define M2_ENA           PIOD
#define M2_ENA_ID        ID_PIOD
#define M2_ENA_IDX       20
#define M2_ENA_IDX_MASK  (1 << M2_ENA_IDX)

#define M2_ENB           PIOD
#define M2_ENB_ID        ID_PIOD
#define M2_ENB_IDX       22
#define M2_ENB_IDX_MASK  (1 << M2_ENB_IDX)



/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	sysclk_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	pmc_enable_periph_clk(M1_A1_ID);
	pio_set_output(M1_A1, M1_A1_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M1_A2_ID);
	pio_set_output(M1_A2, M1_A2_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M1_B1_ID);
	pio_set_output(M1_B1, M1_B1_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M1_B2_ID);
	pio_set_output(M1_B2, M1_B2_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M1_ENA_ID);
	pio_set_output(M1_ENA, M1_ENA_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M1_ENB_ID);
	pio_set_output(M1_ENB, M1_ENB_IDX_MASK, 1, 0, 0);
	
	
	
	
	
	
	
	pmc_enable_periph_clk(M2_A1_ID);
	pio_set_output(M2_A1, M2_A1_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M2_A2_ID);
	pio_set_output(M2_A2, M2_A2_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M2_B1_ID);
	pio_set_output(M2_B1, M2_B1_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M2_B2_ID);
	pio_set_output(M2_B2, M2_B2_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M2_ENA_ID);
	pio_set_output(M2_ENA, M2_ENA_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(M2_ENB_ID);
	pio_set_output(M2_ENB, M2_ENB_IDX_MASK, 1, 0, 0);
	
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	  /*
	 
	pio_set(PIOC, M1_ENA_IDX_MASK);
	pio_set(PIOA, M1_ENB_IDX_MASK);
	
	pio_set(PIOA, M1_A1_IDX_MASK);
	pio_clear(PIOA, M1_A2_IDX_MASK);
	pio_clear(PIOD, M1_B1_IDX_MASK);
	pio_clear(PIOD, M1_B2_IDX_MASK);
	delay_ms(10);
	
	pio_clear(PIOA, M1_A1_IDX_MASK);
	pio_set(PIOA, M1_A2_IDX_MASK);
	pio_clear(PIOD, M1_B1_IDX_MASK);
	pio_clear(PIOD, M1_B2_IDX_MASK);
	delay_ms(10);
	
	pio_clear(PIOA, M1_A1_IDX_MASK);
	pio_clear(PIOA, M1_A2_IDX_MASK);
	pio_clear(PIOD, M1_B1_IDX_MASK);
	pio_set(PIOD, M1_B2_IDX_MASK);
	delay_ms(10);
	
	
	
	pio_clear(PIOA, M1_A1_IDX_MASK);
	pio_clear(PIOA, M1_A2_IDX_MASK);
	pio_set(PIOD, M1_B1_IDX_MASK);
	pio_clear(PIOD, M1_B2_IDX_MASK);
	delay_ms(10);
	
	
	
	
	*/
	
	pio_set(PIOD, M2_ENA_IDX_MASK);
	pio_set(PIOD, M2_ENB_IDX_MASK);
	
	pio_set(PIOA, M2_A1_IDX_MASK);
	pio_clear(PIOC, M2_A2_IDX_MASK);
	pio_clear(PIOD, M2_B1_IDX_MASK);
	pio_clear(PIOD, M2_B2_IDX_MASK);
	delay_ms(10);
	
	pio_clear(PIOA, M2_A1_IDX_MASK);
	pio_set(PIOC, M2_A2_IDX_MASK);
	pio_clear(PIOD, M2_B1_IDX_MASK);
	pio_clear(PIOD, M2_B2_IDX_MASK);
	delay_ms(10);
	
	pio_clear(PIOA, M2_A1_IDX_MASK);
	pio_clear(PIOC, M2_A2_IDX_MASK);
	pio_clear(PIOD, M2_B1_IDX_MASK);
	pio_set(PIOD, M2_B2_IDX_MASK);
	delay_ms(10);
	
	
	
	pio_clear(PIOA, M2_A1_IDX_MASK);
	pio_clear(PIOC, M2_A2_IDX_MASK);
	pio_set(PIOD, M2_B1_IDX_MASK);
	pio_clear(PIOD, M2_B2_IDX_MASK);
	delay_ms(10);
	
	
	
	
  }
  return 0;
}
