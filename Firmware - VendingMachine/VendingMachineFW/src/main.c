#include <asf.h>
#include <filamento.h>
#include <string.h>

// Defines
#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- VENDING MACHINE FW --"STRING_EOL	\
"-- "BOARD_NAME " --"STRING_EOL	\
"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL

#define TASK_LCD_STACK_SIZE       (2048/sizeof(portSTACK_TYPE))
#define TASK_LCD_PRIORITY   (tskIDLE_PRIORITY)
#define TASK_BT_STACK_SIZE       (2048/sizeof(portSTACK_TYPE))
#define TASK_BT_PRIORITY   (tskIDLE_PRIORITY + 1u)
#define TASK_FILAMENTO_STACK_SIZE       (2048/sizeof(portSTACK_TYPE))
#define TASK_FILAMENTO_PRIORITY   (tskIDLE_PRIORITY + 3u)
#define TASK_KEYPAD_STACK_SIZE       (2048/sizeof(portSTACK_TYPE))
#define TASK_KEYPAD_PRIORITY   (tskIDLE_PRIORITY + 3u)

// TODO: Reorganizar parte de motor
#define M1_A1			 PIOA
#define M1_A1_ID        ID_PIOA
#define M1_A1_IDX       5u
#define M1_A1_IDX_MASK  (1u << M1_A1_IDX)

#define M1_A2            PIOA
#define M1_A2_ID        ID_PIOA
#define M1_A2_IDX       6u
#define M1_A2_IDX_MASK  (1u << M1_A2_IDX)

#define M1_B1            PIOD
#define M1_B1_ID        ID_PIOD
#define M1_B1_IDX       27u
#define M1_B1_IDX_MASK  (1u << M1_B1_IDX)

#define M1_B2           PIOD
#define M1_B2_ID        ID_PIOD
#define M1_B2_IDX       11u
#define M1_B2_IDX_MASK  (1u << M1_B2_IDX)

#define M1_ENA           PIOC
#define M1_ENA_ID        ID_PIOC
#define M1_ENA_IDX       19u
#define M1_ENA_IDX_MASK  (1u << M1_ENA_IDX)

#define M1_ENB           PIOA
#define M1_ENB_ID        ID_PIOA
#define M1_ENB_IDX       2u
#define M1_ENB_IDX_MASK  (1u << M1_ENB_IDX)

#define KEYPAD_LINE_1_PIO PIOC
#define KEYPAD_LINE_1 PIO_PC31
#define KEYPAD_LINE_2_PIO PIOB
#define KEYPAD_LINE_2 PIO_PB3
#define KEYPAD_LINE_3_PIO PIOA
#define KEYPAD_LINE_3 PIO_PA0
#define KEYPAD_LINE_4_PIO PIOD
#define KEYPAD_LINE_4 PIO_PD28

#define KEYPAD_COLUMN_1_PIO PIOA
#define KEYPAD_COLUMN_1 PIO_PA19
#define KEYPAD_COLUMN_2_PIO PIOB
#define KEYPAD_COLUMN_2 PIO_PB2
#define KEYPAD_COLUMN_3_PIO PIOC
#define KEYPAD_COLUMN_3 PIO_PC30
#define KEYPAD_COLUMN_4_PIO PIOD
#define KEYPAD_COLUMN_4 PIO_PD20

// Prototype de funçoes
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

// Variaveis globais
struct ili9488_opt_t g_ili9488_display_opt;
SemaphoreHandle_t g_semLCDRedraw;
SemaphoreHandle_t g_semFilamento;
SemaphoreHandle_t g_semKeypad;
char g_opcao[4];
uint32_t g_opcao_num = 0;
uint32_t g_quantidade_num = 0;
char g_quantidade[12];

uint32_t g_valor;
uint32_t g_redraw_all = 0;
uint32_t g_keypad_state = 0;
// Estado 0 -> Opcao
// Estado 1 -> Quantidade
// Estado 2 -> Confirma/Cancela

// Eventos do RTOS
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	* identify which task has overflowed its stack.
	*/
	for (;;) {
	}
}

/**
* \brief This function is called by FreeRTOS idle task
*/
extern void vApplicationIdleHook(void)
{
	//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/**
* \brief This function is called by FreeRTOS each tick
*/
extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}


// UART - Funçoes Genéricas

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, int timeout_ms) {
	long timestart = xTaskGetTickCount();
	uint32_t rx;
	uint32_t counter = 0;
	
	while(xTaskGetTickCount() - timestart < timeout_ms && counter < bufferlen - 1) {
		if(usart_read(usart, &rx) == 0) {
			//timestart = g_systimer; // reset timeout
			buffer[counter++] = rx;
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen, char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_put_string(usart, "\r\n");
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void usart_log(char* name, char* log) {
	usart_put_string(CONF_UART, "[");
	usart_put_string(CONF_UART, name);
	usart_put_string(CONF_UART, "] ");
	usart_put_string(CONF_UART, log);
	usart_put_string(CONF_UART, "\r\n");
}

// UART - Bluetooth - HM10
void hm10_config_server(void) {
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);
	
	// RX - PB0  TX - PB1
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

int hm10_server_init(void) {
	char buffer_rx[128];
	usart_send_command(USART0, buffer_rx, 1000, "AT\r\n", 200);
	usart_send_command(USART0, buffer_rx, 1000, "AT\r\n", 200);
	usart_send_command(USART0, buffer_rx, 1000, "AT\r\n", 200);
	usart_send_command(USART0, buffer_rx, 1000, "AT+RESET\r\n", 400);
	usart_log("hm10_server_init", buffer_rx);
	usart_send_command(USART0, buffer_rx, 1000, "AT+NAMEVending\r\n", 400);
	usart_log("hm10_server_init", buffer_rx);
	usart_send_command(USART0, buffer_rx, 1000, "AT+FILT0\r\n", 400);
	usart_log("hm10_server_init", buffer_rx);
	usart_send_command(USART0, buffer_rx, 1000, "AT+SHOW1\r\n", 400);
	usart_log("hm10_server_init", buffer_rx);
	usart_send_command(USART0, buffer_rx, 1000, "AT+ROLE0\r\n", 400);
	usart_log("hm10_server_init", buffer_rx);
}

// Console - UART
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
	usart_enable_rx(CONF_UART);
	usart_enable_tx(CONF_UART);
}

// LCD
static void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
}

void draw_filament_status(int x, int y, uint32_t color, int filament_id, const char* color_name, int percentage) {
	char percentage_text[6];

	ili9488_draw_pixmap(x, y, 128, 126, image_data_filamento);
	ili9488_set_foreground_color(COLOR_CONVERT(color));
	ili9488_draw_filled_rectangle(x+68, y+60, x+118, y+110);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	ili9488_draw_string(x, y+130, "COR:");
	ili9488_set_foreground_color(COLOR_CONVERT(color));
	ili9488_draw_string(x+50, y+130, color_name);
	if(percentage >= 60) {
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_GREEN));
		} else if(percentage >= 30) {
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_YELLOW));
		} else {
		ili9488_set_foreground_color(COLOR_CONVERT(COLOR_RED));
	}
	ili9488_draw_rectangle(x+10, y+160, x+128, y+160+23);
	ili9488_draw_filled_rectangle(x + 3, y + 160 + 3, x + (128*percentage)/100 - 3, y + 160 + 23 - 3);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	if(abs(percentage) <= 100) {
		sprintf(percentage_text, "%d%%",  percentage);
		} else {
		sprintf(percentage_text, "",  percentage);
	}
	
	ili9488_draw_string(x + 128/2 - 20, y + 160 + 6, percentage_text);
	char buffer[50];
	sprintf(buffer, "OPCAO %d", filament_id);
	ili9488_draw_string(x + 3, y + 190, buffer);
}

// TODO: FAZER INDICACAO DE ESCOLHA DE USUÁRIO
// IRA ESCOLHER VIA KEYPAD
void draw_lcd_screen(void) {
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, 90-1);
	ili9488_draw_filled_rectangle(0, 450, ILI9488_LCD_WIDTH-1, 480-1);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	
	ili9488_draw_string(260, 5, "14:20");
	ili9488_draw_string(10, 5, "LOJA DE FILAMENTOS");
	ili9488_draw_string(10, 25, "- ESCOLHA O NUM. DA OPCAO");
	ili9488_draw_string(10, 45, "- INDIQUE A QUANTIDADE");
	ili9488_draw_string(10, 65, "- PAGUE PELO APP");
	draw_filament_status(10, 100, COLOR_GREEN, 1, "VERDE", 40);
	draw_filament_status(170, 100, COLOR_BLUE, 2, "AZUL", 80);
	
	ili9488_draw_string(5, 435, "* - LIMPAR");
	ili9488_draw_string(160, 435, "# - CONFIRMA");
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_string(5, 455, "DEV BY: LAB DE ARQUITETURA");
	
}

// Motor
// TODO: Reorganizaçao, sincronizar codigo com o mais atual
void motor_config() {
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
}

void motor_passo() {
	static uint32_t passo = 0u;
	
	switch(passo) {
		case 0:
		pio_set(PIOC, M1_ENA_IDX_MASK);
		pio_set(PIOA, M1_ENB_IDX_MASK);
		pio_set(PIOA, M1_A1_IDX_MASK);
		pio_clear(PIOA, M1_A2_IDX_MASK);
		pio_clear(PIOD, M1_B1_IDX_MASK);
		pio_clear(PIOD, M1_B2_IDX_MASK);
		break;
		
		case 1:
		pio_clear(PIOA, M1_A1_IDX_MASK);
		pio_set(PIOA, M1_A2_IDX_MASK);
		pio_clear(PIOD, M1_B1_IDX_MASK);
		pio_clear(PIOD, M1_B2_IDX_MASK);
		break;
		
		case 2:
		pio_clear(PIOA, M1_A1_IDX_MASK);
		pio_clear(PIOA, M1_A2_IDX_MASK);
		pio_clear(PIOD, M1_B1_IDX_MASK);
		pio_set(PIOD, M1_B2_IDX_MASK);
		break;
		
		case 3:
		pio_clear(PIOA, M1_A1_IDX_MASK);
		pio_clear(PIOA, M1_A2_IDX_MASK);
		pio_set(PIOD, M1_B1_IDX_MASK);
		pio_clear(PIOD, M1_B2_IDX_MASK);
		passo = 0u;
		return;
		break;
	}
	passo++;
}


static void taskBluetooth(void *pvParameters) {
	usart_log("BT_Task", "Iniciando...");
	hm10_config_server();
	hm10_server_init();
	usart_log("BT_Task", "Inicializado");
	char buffer[100];
	xSemaphoreGive(g_semLCDRedraw);
	for(;;) {
		if(g_keypad_state < 2) {
			usart_put_string(USART0, "W\n");
			vTaskDelay(5000/portTICK_PERIOD_MS);
		} else if(g_keypad_state == 2) {
			sprintf(buffer, "P;%d;%d;%d\n", atoi(g_opcao), atoi(g_quantidade), 150);
			usart_put_string(USART0, buffer);
			
			uint32_t read = usart_get_string(USART0, buffer, 100, 1000);
			usart_log("bt", buffer);
			if(read > 0 && strstr(buffer, "A") != 0) {
				usart_put_string(USART0, "V\n");
			}
			vTaskDelay(200/portTICK_PERIOD_MS);
		}
	}
}

static void taskLCD(void *pvParameters) {
	usart_log("LCD_Task", "Iniciando...");
	configure_lcd();
	draw_lcd_screen();
	usart_log("BT_Task", "Iniciado");
	sprintf(g_opcao, "");
	sprintf(g_quantidade, "");
	g_valor = 0;
	char buffer[12];
	for(;;) {
		if( xSemaphoreTake(g_semLCDRedraw, 2000))
		{
			if(g_redraw_all) {
				draw_lcd_screen();
			}
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
			ili9488_draw_filled_rectangle(0, 310, ILI9488_LCD_WIDTH-1, 430);
			ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
			if(g_keypad_state == 0) {
				ili9488_draw_string(80, 370, "DIGITE A OPCAO");
				ili9488_draw_string(160, 390, g_opcao);
			} else if(g_keypad_state == 1) {
				ili9488_draw_string(20, 370, "DIGITE A QUANTIDADE (CM)");
				ili9488_draw_string(160, 390, g_quantidade);
			} else if(g_keypad_state == 2) {
				ili9488_draw_string(80, 370, "PAGUE PELO APP");
				sprintf(buffer, "R$%d,00", g_valor);
				ili9488_draw_string(120, 390, buffer);
			}
		}
	}
}




void keypad_ir(void) {
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	xSemaphoreGiveFromISR(g_semKeypad, &xHigherPriorityTaskWoken);
}

static void configure_keypad(void) {
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	NVIC_EnableIRQ(ID_PIOA);
	NVIC_SetPriority(ID_PIOA, 5);
	
	NVIC_EnableIRQ(ID_PIOB);
	NVIC_SetPriority(ID_PIOB, 5);
	
	NVIC_EnableIRQ(ID_PIOC);
	NVIC_SetPriority(ID_PIOC, 5);
		
	NVIC_EnableIRQ(ID_PIOD);
	NVIC_SetPriority(ID_PIOD, 5);
	
	pio_configure(KEYPAD_LINE_1_PIO, PIO_OUTPUT_0, KEYPAD_LINE_1, PIO_DEFAULT);
	pio_configure(KEYPAD_COLUMN_1_PIO, PIO_INPUT, KEYPAD_COLUMN_1, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(KEYPAD_LINE_2_PIO, PIO_OUTPUT_0, KEYPAD_LINE_2, PIO_DEFAULT);
	pio_configure(KEYPAD_COLUMN_2_PIO, PIO_INPUT, KEYPAD_COLUMN_2, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(KEYPAD_LINE_3_PIO, PIO_OUTPUT_0, KEYPAD_LINE_3, PIO_DEFAULT);
	pio_configure(KEYPAD_COLUMN_3_PIO, PIO_INPUT, KEYPAD_COLUMN_3, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(KEYPAD_LINE_4_PIO, PIO_OUTPUT_0, KEYPAD_LINE_4, PIO_DEFAULT);
	pio_configure(KEYPAD_COLUMN_4_PIO, PIO_INPUT, KEYPAD_COLUMN_4, PIO_PULLUP | PIO_DEBOUNCE);
	
	pio_set_debounce_filter(KEYPAD_COLUMN_1_PIO, KEYPAD_COLUMN_1, 200);
	pio_set_debounce_filter(KEYPAD_COLUMN_2_PIO, KEYPAD_COLUMN_2, 200);
	pio_set_debounce_filter(KEYPAD_COLUMN_3_PIO, KEYPAD_COLUMN_3, 200);
	pio_set_debounce_filter(KEYPAD_COLUMN_4_PIO, KEYPAD_COLUMN_4, 200);
	
	pio_enable_interrupt(KEYPAD_COLUMN_1_PIO, KEYPAD_COLUMN_1);
	pio_enable_interrupt(KEYPAD_COLUMN_2_PIO, KEYPAD_COLUMN_2);
	pio_enable_interrupt(KEYPAD_COLUMN_3_PIO, KEYPAD_COLUMN_3);
	pio_enable_interrupt(KEYPAD_COLUMN_4_PIO, KEYPAD_COLUMN_4);
	
	pio_handler_set(KEYPAD_COLUMN_1_PIO, ID_PIOA, KEYPAD_COLUMN_1, PIO_IT_FALL_EDGE,  keypad_ir);
	pio_handler_set(KEYPAD_COLUMN_2_PIO, ID_PIOB, KEYPAD_COLUMN_2, PIO_IT_FALL_EDGE,  keypad_ir);
	pio_handler_set(KEYPAD_COLUMN_3_PIO, ID_PIOC, KEYPAD_COLUMN_3, PIO_IT_FALL_EDGE,  keypad_ir);
	pio_handler_set(KEYPAD_COLUMN_4_PIO, ID_PIOD, KEYPAD_COLUMN_4, PIO_IT_FALL_EDGE,  keypad_ir);
	
}

static void taskFilamento(void *pvParameters) {
	usart_log("Filamento_Task", "Iniciando...");
	for(;;) {
		if( xSemaphoreTake(g_semFilamento, 10))
		{
			for(uint32_t i = 0; i < 10; i++) {
				motor_passo();
				vTaskDelay(10/portTICK_PERIOD_MS);
			}
			} else {
			vTaskDelay(4000/portTICK_PERIOD_MS);
		}
	}
}

static int keypad_check_pin(uint32_t line_pio, uint32_t line, uint32_t column_pio, uint32_t column) {
	pio_clear(line_pio, line);
	
	vTaskDelay(10/portTICK_PERIOD_MS); // estabilizar linha
	int val = pio_get(column_pio, PIO_INPUT, column) == LOW;
	// espera soltar
	while (pio_get(column_pio, PIO_INPUT, column) == LOW) {
		vTaskDelay(200/portTICK_PERIOD_MS);
	}
	return val;
}

static char keypad_get_value() {
	uint32_t KEYPAD_LINE_PIO_ARRAY[] = {KEYPAD_LINE_1_PIO, KEYPAD_LINE_2_PIO, KEYPAD_LINE_3_PIO, KEYPAD_LINE_4_PIO};
	uint32_t KEYPAD_LINE_ARRAY[] = {KEYPAD_LINE_1, KEYPAD_LINE_2, KEYPAD_LINE_3, KEYPAD_LINE_4};
	uint32_t KEYPAD_COLUMN_PIO_ARRAY[] = {KEYPAD_COLUMN_1_PIO, KEYPAD_COLUMN_2_PIO, KEYPAD_COLUMN_3_PIO, KEYPAD_COLUMN_3_PIO}; //KEYPAD_COLUMN_4_PIO
	uint32_t KEYPAD_COLUMN_ARRAY[] = {KEYPAD_COLUMN_1, KEYPAD_COLUMN_2, KEYPAD_COLUMN_3, KEYPAD_COLUMN_3}; //KEYPAD_COLUMN_4
	char KEYPAD_OUTPUT_ARRAY[] = {'1', '2', '3', 'A', '4', '5','6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };
	
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			pio_set(KEYPAD_LINE_1_PIO, KEYPAD_LINE_1);
			pio_set(KEYPAD_LINE_2_PIO, KEYPAD_LINE_2);
			pio_set(KEYPAD_LINE_3_PIO, KEYPAD_LINE_3);
			pio_set(KEYPAD_LINE_4_PIO, KEYPAD_LINE_4);
			if(keypad_check_pin(KEYPAD_LINE_PIO_ARRAY[i], KEYPAD_LINE_ARRAY[i], KEYPAD_COLUMN_PIO_ARRAY[j],  KEYPAD_COLUMN_ARRAY[j])) {
				return KEYPAD_OUTPUT_ARRAY[i*4+j];
			}
		}
	}
	
	return ' ';
}


static void taskKeypad(void *pvParameters) {
	configure_keypad();
	usart_log("Keypad_task", "Iniciando...");
	
	for(;;) {
		if(xSemaphoreTake(g_semKeypad, 10000) == pdTRUE) {
			char k = keypad_get_value();
			switch(g_keypad_state) {
				case 0:
					if(k == '*') {
						sprintf(g_opcao, "");
						g_opcao_num = 0;
					} else if (k == '#') {
						g_keypad_state = 1;
					} else if(g_opcao_num < 1 && k != ' ') {
						g_opcao[g_opcao_num++] = k;
						g_opcao[g_opcao_num] = '\0';
					}
				break;
				
				case 1:
					if(k == '*') {
						sprintf(g_quantidade, "");
						g_quantidade_num = 0;
					} else if (k == '#') {
						g_keypad_state = 2;
					} else if(g_quantidade_num < 4 && k != ' ') {
						g_quantidade[g_quantidade_num++] = k;
						g_quantidade[g_quantidade_num] = '\0';
					}
				break;
				
				case 2:
					if(k == '*') {
						sprintf(g_quantidade, "");
						g_quantidade_num = 0;
						sprintf(g_opcao, "");
						g_opcao_num = 0;
						g_keypad_state = 0;
					}
				break;
				
			}
			pio_clear(KEYPAD_LINE_1_PIO, KEYPAD_LINE_1);
			pio_clear(KEYPAD_LINE_2_PIO, KEYPAD_LINE_2);
			pio_clear(KEYPAD_LINE_3_PIO, KEYPAD_LINE_3);
			pio_clear(KEYPAD_LINE_4_PIO, KEYPAD_LINE_4);
			xSemaphoreGive(g_semLCDRedraw);
		}
	}
}

// Start
int main (void)
{
	sysclk_init();
	board_init();
	ioport_init();
	configure_console();
	
	printf(STRING_HEADER);
	
	g_semLCDRedraw = xSemaphoreCreateBinary();
	if (g_semLCDRedraw == NULL) {
		printf("falha em criar o semaforo \n");
	}
	
	g_semFilamento = xSemaphoreCreateBinary();
	if (g_semFilamento == NULL) {
		printf("falha em criar o semaforo \n");
	}
	
	g_semKeypad = xSemaphoreCreateBinary();
	if (g_semKeypad == NULL) {
		printf("falha em criar o semaforo \n");
	}
	
	if (xTaskCreate(taskBluetooth, "Bluetooth", TASK_BT_STACK_SIZE, NULL, TASK_BT_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create BT task\r\n");
	}

	if (xTaskCreate(taskLCD, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create LCD task\r\n");
	}
	
	if (xTaskCreate(taskFilamento, "Filamento", TASK_FILAMENTO_STACK_SIZE, NULL, TASK_FILAMENTO_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Filamento task\r\n");
	}
	
	if (xTaskCreate(taskKeypad, "Keypad", TASK_KEYPAD_STACK_SIZE, NULL, TASK_KEYPAD_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Filamento task\r\n");
	}
	
	vTaskStartScheduler();
	while(1) {};
	return 0;
}