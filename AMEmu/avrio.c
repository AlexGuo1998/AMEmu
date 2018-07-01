#include "avrio.h"

#include <stdio.h>
#include <stdlib.h>

void DebugPrint(const char *str) {
	fputs(str, stderr);
}

void IOWriteDefault(AVRMCU *u, uint8_t addr, uint8_t data) {
	char buffer[128];
	snprintf(buffer, 128, "IO Port 0x%02hhX(mem 0x%02hhX) not implemented for write!\n", addr - 0x20, addr);
	DebugPrint(buffer);
}

uint8_t IOReadDefault(AVRMCU *u, uint8_t addr) {
	char buffer[128];
	snprintf(buffer, 128, "IO Port 0x%02hhX(mem 0x%02hhX) not implemented for read!\n", addr - 0x20, addr);
	DebugPrint(buffer);
	return 0;
}

void IOWriteReserved(AVRMCU *u, uint8_t addr, uint8_t data) {
	char buffer[128];
	snprintf(buffer, 128, "IO Port 0x%02hhX(mem 0x%02hhX) is reserved and should not be used for write!\n", addr - 0x20, addr);
	DebugPrint(buffer);
}

uint8_t IOReadReserved(AVRMCU *u, uint8_t addr) {
	return 0;
}

const IOWriteCallback IOWriteCallbackDefault[0x100 - 0x20] = {
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //PINB
	IOWriteDefault, //DDRB
	IOWriteDefault, //PORTB
	IOWriteDefault, //PINC
	IOWriteDefault, //DDRC
	IOWriteDefault, //PORTC
	IOWriteDefault, //PIND
	IOWriteDefault, //DDRD
	IOWriteDefault, //PORTD
	IOWriteDefault, //PINE
	IOWriteDefault, //DDRE
	IOWriteDefault, //PORTE
	IOWriteDefault, //PINF

	IOWriteDefault, //DDRF
	IOWriteDefault, //PORTF
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //TIFR0
	IOWriteDefault, //TIFR1
	IOWriteReserved,
	IOWriteDefault, //TIFR3
	IOWriteDefault, //TIFR4
	IOWriteReserved,
	IOWriteDefault, //PCIFR
	IOWriteDefault, //EIFR
	IOWriteDefault, //EIMSK
	NULL, //GPIOR0
	IOWriteDefault, //EECR

	IOWriteDefault, //EEDR
	IOWriteDefault, //EEARL
	IOWriteDefault, //EEARH
	IOWriteDefault, //GTCCR
	IOWriteDefault, //TCCR0A
	IOWriteDefault, //TCCR0B
	IOWriteDefault, //TCNT0
	IOWriteDefault, //OCR0A
	IOWriteDefault, //OCR0B
	IOWriteDefault, //PLLCSR
	NULL, //GPIOR1
	NULL, //GPIOR2
	IOWriteDefault, //SPCR
	IOWriteDefault, //SPSR
	IOWriteDefault, //SPDR
	IOWriteReserved,

	IOWriteDefault, //ACSR
	IOWriteDefault, //OCDR/MONDR
	IOWriteDefault, //PLLFRQ
	IOWriteDefault, //SMCR
	IOWriteDefault, //MCUSR
	IOWriteDefault, //MCUCR
	IOWriteReserved,
	IOWriteDefault, //SPMCSR
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //RAMPZ
	IOWriteReserved,
	NULL, //SPL
	NULL, //SPH
	NULL, //SREG

	//memory addressing only
	IOWriteDefault, //WDTCSR
	IOWriteDefault, //CLKPR
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //PRR0
	IOWriteDefault, //PRR1
	IOWriteDefault, //OSCCAL
	IOWriteDefault, //RCCTRL
	IOWriteDefault, //PCICR
	IOWriteDefault, //EICRA
	IOWriteDefault, //EICRB
	IOWriteDefault, //PCMSK0
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //TIMSK0
	IOWriteDefault, //TIMSK1

	IOWriteReserved,
	IOWriteDefault, //TIMSK3
	IOWriteDefault, //TIMSK4
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //ADCL
	IOWriteDefault, //ADCH
	IOWriteDefault, //ADCSRA
	IOWriteDefault, //ADCSRB
	IOWriteDefault, //ADMUX
	IOWriteDefault, //DIDR2
	IOWriteDefault, //DIDR0
	IOWriteDefault, //DIDR1

	IOWriteDefault, //TCCR1A
	IOWriteDefault, //TCCR1B
	IOWriteDefault, //TCCR1C
	IOWriteReserved,
	IOWriteDefault, //TCNT1L
	IOWriteDefault, //TCNT1H
	IOWriteDefault, //ICR1L
	IOWriteDefault, //ICR1H
	IOWriteDefault, //OCR1AL
	IOWriteDefault, //OCR1AH
	IOWriteDefault, //OCR1BL
	IOWriteDefault, //OCR1BH
	IOWriteDefault, //OCR1CL
	IOWriteDefault, //OCR1CH
	IOWriteReserved,
	IOWriteReserved,

	IOWriteDefault, //TCCR3A
	IOWriteDefault, //TCCR3B
	IOWriteDefault, //TCCR3C
	IOWriteReserved,
	IOWriteDefault, //TCNT3L
	IOWriteDefault, //TCNT3H
	IOWriteDefault, //ICR3L
	IOWriteDefault, //ICR3H
	IOWriteDefault, //OCR3AL
	IOWriteDefault, //OCR3AH
	IOWriteDefault, //OCR3BL
	IOWriteDefault, //OCR3BH
	IOWriteDefault, //OCR3CL
	IOWriteDefault, //OCR3CH
	IOWriteReserved,
	IOWriteReserved,

	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,

	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //TWBR
	IOWriteDefault, //TWSR
	IOWriteDefault, //TWAR
	IOWriteDefault, //TWDR
	IOWriteDefault, //TWCR
	IOWriteDefault, //TWAMR
	IOWriteDefault, //TCNT4
	IOWriteDefault, //TC4H

	IOWriteDefault, //TCCR4A
	IOWriteDefault, //TCCR4B
	IOWriteDefault, //TCCR4C
	IOWriteDefault, //TCCR4D
	IOWriteDefault, //TCCR4E
	IOWriteDefault, //CLKSEL0
	IOWriteDefault, //CLKSEL1
	IOWriteDefault, //CLKSTA
	IOWriteDefault, //UCSR1A
	IOWriteDefault, //UCSR1B
	IOWriteDefault, //UCSR1C
	IOWriteDefault, //UCSR1D
	IOWriteDefault, //UBRR1L
	IOWriteDefault, //UBRR1H
	IOWriteDefault, //UDR1
	IOWriteDefault, //OCR4A

	IOWriteDefault, //OCR4B
	IOWriteDefault, //OCR4C
	IOWriteDefault, //OCR4D
	IOWriteReserved,
	IOWriteDefault, //DT4
	IOWriteReserved,
	IOWriteReserved,
	IOWriteDefault, //UHWCON
	IOWriteDefault, //USBCON
	IOWriteDefault, //USBSTA
	IOWriteDefault, //USBINT
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,

	IOWriteDefault, //UDCON
	IOWriteDefault, //UDINT
	IOWriteDefault, //UDIEN
	IOWriteDefault, //UDADDR
	IOWriteDefault, //UDFNUML
	IOWriteDefault, //UDFNUMH
	IOWriteDefault, //UDMFN
	IOWriteReserved,
	IOWriteDefault, //UEINTX
	IOWriteDefault, //UENUM
	IOWriteDefault, //UERST
	IOWriteDefault, //UECONX
	IOWriteDefault, //UECFG0X
	IOWriteDefault, //UECFG1X
	IOWriteDefault, //UESTA0X
	IOWriteDefault, //UESTA1X

	IOWriteDefault, //UEIENX
	IOWriteDefault, //UEDATA
	IOWriteDefault, //UEBCLX
	IOWriteDefault, //UEBCHX
	IOWriteDefault, //UEINT
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
	IOWriteReserved,
};

const IOReadCallback IOReadCallbackDefault[0x100 - 0x20] = {
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //PINB
	IOReadDefault, //DDRB
	IOReadDefault, //PORTB
	IOReadDefault, //PINC
	IOReadDefault, //DDRC
	IOReadDefault, //PORTC
	IOReadDefault, //PIND
	IOReadDefault, //DDRD
	IOReadDefault, //PORTD
	IOReadDefault, //PINE
	IOReadDefault, //DDRE
	IOReadDefault, //PORTE
	IOReadDefault, //PINF

	IOReadDefault, //DDRF
	IOReadDefault, //PORTF
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //TIFR0
	IOReadDefault, //TIFR1
	IOReadReserved,
	IOReadDefault, //TIFR3
	IOReadDefault, //TIFR4
	IOReadReserved,
	IOReadDefault, //PCIFR
	IOReadDefault, //EIFR
	IOReadDefault, //EIMSK
	NULL, //GPIOR0
	IOReadDefault, //EECR

	IOReadDefault, //EEDR
	IOReadDefault, //EEARL
	IOReadDefault, //EEARH
	IOReadDefault, //GTCCR
	IOReadDefault, //TCCR0A
	IOReadDefault, //TCCR0B
	IOReadDefault, //TCNT0
	IOReadDefault, //OCR0A
	IOReadDefault, //OCR0B
	IOReadDefault, //PLLCSR
	NULL, //GPIOR1
	NULL, //GPIOR2
	IOReadDefault, //SPCR
	IOReadDefault, //SPSR
	IOReadDefault, //SPDR
	IOReadReserved,

	IOReadDefault, //ACSR
	IOReadDefault, //OCDR/MONDR
	IOReadDefault, //PLLFRQ
	IOReadDefault, //SMCR
	IOReadDefault, //MCUSR
	IOReadDefault, //MCUCR
	IOReadReserved,
	IOReadDefault, //SPMCSR
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //RAMPZ
	IOReadReserved,
	NULL, //SPL
	NULL, //SPH
	NULL, //SREG

	//memory addressing only
	IOReadDefault, //WDTCSR
	IOReadDefault, //CLKPR
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //PRR0
	IOReadDefault, //PRR1
	IOReadDefault, //OSCCAL
	IOReadDefault, //RCCTRL
	IOReadDefault, //PCICR
	IOReadDefault, //EICRA
	IOReadDefault, //EICRB
	IOReadDefault, //PCMSK0
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //TIMSK0
	IOReadDefault, //TIMSK1

	IOReadReserved,
	IOReadDefault, //TIMSK3
	IOReadDefault, //TIMSK4
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //ADCL
	IOReadDefault, //ADCH
	IOReadDefault, //ADCSRA
	IOReadDefault, //ADCSRB
	IOReadDefault, //ADMUX
	IOReadDefault, //DIDR2
	IOReadDefault, //DIDR0
	IOReadDefault, //DIDR1

	IOReadDefault, //TCCR1A
	IOReadDefault, //TCCR1B
	IOReadDefault, //TCCR1C
	IOReadReserved,
	IOReadDefault, //TCNT1L
	IOReadDefault, //TCNT1H
	IOReadDefault, //ICR1L
	IOReadDefault, //ICR1H
	IOReadDefault, //OCR1AL
	IOReadDefault, //OCR1AH
	IOReadDefault, //OCR1BL
	IOReadDefault, //OCR1BH
	IOReadDefault, //OCR1CL
	IOReadDefault, //OCR1CH
	IOReadReserved,
	IOReadReserved,

	IOReadDefault, //TCCR3A
	IOReadDefault, //TCCR3B
	IOReadDefault, //TCCR3C
	IOReadReserved,
	IOReadDefault, //TCNT3L
	IOReadDefault, //TCNT3H
	IOReadDefault, //ICR3L
	IOReadDefault, //ICR3H
	IOReadDefault, //OCR3AL
	IOReadDefault, //OCR3AH
	IOReadDefault, //OCR3BL
	IOReadDefault, //OCR3BH
	IOReadDefault, //OCR3CL
	IOReadDefault, //OCR3CH
	IOReadReserved,
	IOReadReserved,

	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,

	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //TWBR
	IOReadDefault, //TWSR
	IOReadDefault, //TWAR
	IOReadDefault, //TWDR
	IOReadDefault, //TWCR
	IOReadDefault, //TWAMR
	IOReadDefault, //TCNT4
	IOReadDefault, //TC4H

	IOReadDefault, //TCCR4A
	IOReadDefault, //TCCR4B
	IOReadDefault, //TCCR4C
	IOReadDefault, //TCCR4D
	IOReadDefault, //TCCR4E
	IOReadDefault, //CLKSEL0
	IOReadDefault, //CLKSEL1
	IOReadDefault, //CLKSTA
	IOReadDefault, //UCSR1A
	IOReadDefault, //UCSR1B
	IOReadDefault, //UCSR1C
	IOReadDefault, //UCSR1D
	IOReadDefault, //UBRR1L
	IOReadDefault, //UBRR1H
	IOReadDefault, //UDR1
	IOReadDefault, //OCR4A

	IOReadDefault, //OCR4B
	IOReadDefault, //OCR4C
	IOReadDefault, //OCR4D
	IOReadReserved,
	IOReadDefault, //DT4
	IOReadReserved,
	IOReadReserved,
	IOReadDefault, //UHWCON
	IOReadDefault, //USBCON
	IOReadDefault, //USBSTA
	IOReadDefault, //USBINT
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,

	IOReadDefault, //UDCON
	IOReadDefault, //UDINT
	IOReadDefault, //UDIEN
	IOReadDefault, //UDADDR
	IOReadDefault, //UDFNUML
	IOReadDefault, //UDFNUMH
	IOReadDefault, //UDMFN
	IOReadReserved,
	IOReadDefault, //UEINTX
	IOReadDefault, //UENUM
	IOReadDefault, //UERST
	IOReadDefault, //UECONX
	IOReadDefault, //UECFG0X
	IOReadDefault, //UECFG1X
	IOReadDefault, //UESTA0X
	IOReadDefault, //UESTA1X

	IOReadDefault, //UEIENX
	IOReadDefault, //UEDATA
	IOReadDefault, //UEBCLX
	IOReadDefault, //UEBCHX
	IOReadDefault, //UEINT
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
	IOReadReserved,
};
