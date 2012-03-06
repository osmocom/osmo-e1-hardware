#include <stdint.h>

#include <spi/spi.h>

#include "idt82v2081.h"

/* Adaption layer between idt82 driver and at91lib SPI driver */

#define B_READ	(1 << 5)

/* backend function for core idt82 driver */
int idt82_reg_read(struct idt82 *idt, uint8_t reg)
{
	uint16_t res;

	SPI_Write(idt->priv, idt->cs, (reg & 0x1F) | B_READ);
	while (!SPI_IsFinished(idt->priv));
	res = SPI_Read(idt->priv, idt->cs);

	return res >> 8;
}
/* backend function for core idt82 driver */
int idt82_reg_write(struct idt82 *idt, uint8_t reg, uint8_t val)
{
	SPI_Write(idt->priv, idt->cs, (reg & 0x1F) | B_READ | (val << 8));

	return 0;
}

/* initialize the SPI interface to the IDT82 */
int idt82_at91_init(struct idt82 *idt, void *spi, unsigned int id,
		    uint8_t cs, uint32_t spi_mr, uint32_t csr)
{
	idt->priv = spi;

	SPI_Configure(spi, id, spi_mr);
	SPI_ConfigureNCPS(spi, cs, csr);
	SPI_Enable(spi);
}
