#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"

void measure_freqs(void)
{ // from Hardvare APIs/hardware_Clock/Example
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);

    printf("pll_sys  = %u kHz\n", f_pll_sys);    // A phase-locked loop that generates a higher frequency clock used to drive the 
                                                //   clk_sys (system clock) and other internal components of the microcontroller.
    printf("pll_usb  = %u kHz\n", f_pll_usb);    // A phase-locked loop dedicated to generating a stable 48 MHz clock used to drive 
                                                //   the clk_usb for USB functionality.
    printf("rosc     = %u kHz\n", f_rosc);       // An internal low-precision oscillator that can serve as a backup clock when needed.
    printf("clk_sys  = %u kHz\n", f_clk_sys);    // The main system clock that drives the processor and most of the internal components,
                                                //   typically sourced from a PLL.  
    printf("clk_ref  = %d kHz\n", clock_get_hz(clk_ref) / 1000);
    printf("clk_peri = %u kHz\n", f_clk_peri);   // The clock for peripherals like UART, SPI, and I2C, which must operate at a stable 
                                                //    frequency for reliable communication.
    printf("clk_usb  = %u kHz\n", f_clk_usb);    // The clock used to drive USB functionality, typically set to 48 MHz.
    printf("clk_adc  = %u kHz\n", f_clk_adc);    // The clock used by the analog-to-digital converter (ADC) for accurate sampling.
    
}

void configure_clocks() {
    //pico-sdk /src/rp2350/hardware_regs/include/hardware/regs/clocks.h
    clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0, 12 * 1000000, 12 * 1000000);
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_XOSC_CLKSRC, 12 * 1000000, 12 * 1000000);
}

void choice0()
{   
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, 12 * 1000000, 12 * 1000000); // Hadware APIs/hardware_clocks/Main Clock Sources
    pll_deinit(pll_sys); // Hardware APIs/hardware_pll
    stdio_init_all();
}

void choice1()
{
     clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, 12 * 1000000, 12 * 1000000); // Hadware APIs/hardware_clocks/Main Clock Sources
    
//651 PLL_SYS 150 MHz
    pll_init(pll_sys, 1, 900 * 1000000, 6, 1); //   (clock, external clock, ref_freq, dividiv , jyter)

    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX, //651
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, //633
                    150 * 1000000,
                    150 * 1000000);
    stdio_init_all();
}

void choice2()
{
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, 12 * 1000000, 12 * 1000000);

    pll_init(pll_sys, 1, 600 * 1000000, 6, 1); //   (clock, external clock, ref_freq, dividiv , jyter)

    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX, //651
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, //633
                    100 * 1000000,
                    100 * 1000000);
    stdio_init_all();
}

void choice3()
{
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0, 12 * 1000000, 12 * 1000000);

    pll_init(pll_sys, 1, 600 * 1000000, 6, 2); //   (clock, external clock, ref_freq, dividiv , jyter)

    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX, //651
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, //633
                    50 * 1000000,
                    50 * 1000000);
    stdio_init_all();
}
int main()
{
    configure_clocks();
    stdio_init_all();
    measure_freqs();
   
    while (true)
    {
        printf("\nEnter a number 0 to 3: \n");
        int choice;
        scanf("%d", &choice);
        printf("\nYour choice is %d\n", choice);

        switch (choice) {
        case 0:
            choice0();
            measure_freqs();
            break;
        case 1:
            choice1();
            measure_freqs();
            break;
        case 2:
            choice2();
            measure_freqs();
            break;
        case 3:
            choice3();
            measure_freqs();
            break;
        default:
            printf("Wrong choice\n");
            break;
        }
    }
    return 0;
}
