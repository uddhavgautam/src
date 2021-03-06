/* $OpenBSD: acpimcfg.c,v 1.3 2018/07/04 20:46:22 kettenis Exp $ */
/*
 * Copyright (c) 2010 Mark Kettenis <kettenis@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <dev/acpi/acpireg.h>
#include <dev/acpi/acpivar.h>
#include <dev/pci/pcivar.h>

int acpimcfg_match(struct device *, void *, void *);
void acpimcfg_attach(struct device *, struct device *, void *);

struct cfattach acpimcfg_ca = {
	sizeof(struct device), acpimcfg_match, acpimcfg_attach
};

struct cfdriver acpimcfg_cd = {
	NULL, "acpimcfg", DV_DULL
};

int
acpimcfg_match(struct device *parent, void *match, void *aux)
{
	struct acpi_attach_args *aaa = aux;
	struct acpi_table_header *hdr;

	/*
	 * If we do not have a table, it is not us
	 */
	if (aaa->aaa_table == NULL)
		return (0);

	/*
	 * If it is an MCFG table, we can attach
	 */
	hdr = (struct acpi_table_header *)aaa->aaa_table;
	if (memcmp(hdr->signature, MCFG_SIG, sizeof(MCFG_SIG) - 1) != 0)
		return (0);

	return (1);
}

void
acpimcfg_attach(struct device *parent, struct device *self, void *aux)
{
	struct acpi_softc *sc = (struct acpi_softc *)parent;
	struct acpi_attach_args *aaa = aux;
	struct acpi_mcfg *mcfg = (struct acpi_mcfg *)aaa->aaa_table;

	printf(" addr 0x%llx, bus %d-%d\n", mcfg->base_address,
	    mcfg->min_bus_number, mcfg->max_bus_number);

	/*
	 * Some (broken?) BIOSen have an MCFG table for an empty bus
	 * range.  Ignore those tables.
	 */
	if (mcfg->min_bus_number == mcfg->max_bus_number)
		return;

	sc->sc_pc = pci_mcfg_init(aaa->aaa_memt, mcfg->base_address,
	    mcfg->min_bus_number, mcfg->max_bus_number);
}
