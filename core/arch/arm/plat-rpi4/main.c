// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2016, Sequitur Labs Inc. All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <console.h>
#include <drivers/serial8250_uart.h>
#include <kernel/panic.h>
#include <mm/core_memprot.h>
#include <mm/tee_pager.h>
#include <platform_config.h>
#include <stdint.h>

register_phys_mem_pgdir(MEM_AREA_IO_NSEC,
			CONSOLE_UART_BASE, SERIAL8250_UART_REG_SIZE);

static struct serial8250_uart_data console_data;

void putStr(const char *str);
static void setPutCh(void);

void console_init(void)
{
	serial8250_uart_init(&console_data, CONSOLE_UART_BASE,
			     CONSOLE_UART_CLK_IN_HZ, CONSOLE_BAUDRATE);
	register_serial_console(&console_data.chip);

	putStr("optee-os: did serial8250_uart_init work?\n");
	setPutCh();
	putStr("optee-os: console_init done\n");
}


static void putCh(char c)
{
  console_data.chip.ops->putc(&console_data.chip, c);
}

void putStr(const char *str)
{
  while (*str)
  {
    if (*str == '\n')
      putCh('\r');
    putCh(*str++);
  }
}

void putStrReturn(void);
void putStrReturn(void) {
  putStr("optee-os: return\n");
}
void putStrCheckpoint1(void);
void putStrCheckpoint1(void) {
  putStr("optee-os: checkpoint 1\n");
}

enum
{
  PERIPHERAL_BASE = 0xFE000000,
  AUX_BASE        = PERIPHERAL_BASE + 0x215000,
  AUX_MU_IO_REG   = AUX_BASE + 64,
  AUX_MU_LSR_REG  = AUX_BASE + 84,
};

static uint32_t mmioRead(int64_t reg)
{
  return *(volatile uint32_t*)reg;
}

static void mmioWrite(int64_t reg, uint32_t val)
{
  *(volatile uint32_t*)reg = val;
}

static uint32_t uartIsWriteCharReady(void)
{
  return mmioRead(AUX_MU_LSR_REG) & 0x20;
}

static void uartWriteChar(char c)
{
  while (!uartIsWriteCharReady());
  mmioWrite(AUX_MU_IO_REG, c);
}

static void _putc(struct serial_chip *dummy __maybe_unused, int c)
{
  uartWriteChar(c);
}

static void setPutCh(void)
{
  static struct serial_ops ops;
  ops.putc         = _putc;
  ops.flush        = console_data.chip.ops->flush;
  ops.have_rx_data = console_data.chip.ops->have_rx_data;
  ops.getchar      = console_data.chip.ops->getchar;
  console_data.chip.ops = &ops;
}
