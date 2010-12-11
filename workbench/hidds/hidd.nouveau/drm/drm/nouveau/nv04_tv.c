/*
 * Copyright (C) 2009 Francisco Jerez.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "drmP.h"
#include "nouveau_drv.h"
#include "nouveau_encoder.h"
#include "nouveau_connector.h"
#include "nouveau_crtc.h"
#include "nouveau_hw.h"
#include "drm_crtc_helper.h"

#if !defined(__AROS__)
#include "i2c/ch7006.h"

static struct i2c_board_info nv04_tv_encoder_info[] = {
	{
		I2C_BOARD_INFO("ch7006", 0x75),
		.platform_data = &(struct ch7006_encoder_params) {
			CH7006_FORMAT_RGB24m12I, CH7006_CLOCK_MASTER,
			0, 0, 0,
			CH7006_SYNC_SLAVE, CH7006_SYNC_SEPARATED,
			CH7006_POUT_3_3V, CH7006_ACTIVE_HSYNC
		}
	},
	{ }
};
#else
static struct i2c_board_info nv04_tv_encoder_info[] = {
    { .type = "ch7006", .addr = 0x75 },
	{ }
};
#endif

int nv04_tv_identify(struct drm_device *dev, int i2c_index)
{
	return nouveau_i2c_identify(dev, "TV encoder", nv04_tv_encoder_info,
				    NULL, i2c_index);
}

#define PLLSEL_TV_CRTC1_MASK				\
	(NV_PRAMDAC_PLL_COEFF_SELECT_TV_VSCLK1		\
	 | NV_PRAMDAC_PLL_COEFF_SELECT_TV_PCLK1)
#define PLLSEL_TV_CRTC2_MASK				\
	(NV_PRAMDAC_PLL_COEFF_SELECT_TV_VSCLK2		\
	 | NV_PRAMDAC_PLL_COEFF_SELECT_TV_PCLK2)

static void nv04_tv_dpms(struct drm_encoder *encoder, int mode)
{
	struct drm_device *dev = encoder->dev;
	struct nouveau_encoder *nv_encoder = nouveau_encoder(encoder);
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct nv04_mode_state *state = &dev_priv->mode_reg;
	uint8_t crtc1A;

	NV_INFO(dev, "Setting dpms mode %d on TV encoder (output %d)\n",
		mode, nv_encoder->dcb->index);

	state->pllsel &= ~(PLLSEL_TV_CRTC1_MASK | PLLSEL_TV_CRTC2_MASK);

	if (mode == DRM_MODE_DPMS_ON) {
		int head = nouveau_crtc(encoder->crtc)->index;
		crtc1A = NVReadVgaCrtc(dev, head, NV_CIO_CRE_RPC1_INDEX);

		state->pllsel |= head ? PLLSEL_TV_CRTC2_MASK :
					PLLSEL_TV_CRTC1_MASK;

		/* Inhibit hsync */
		crtc1A |= 0x80;

		NVWriteVgaCrtc(dev, head, NV_CIO_CRE_RPC1_INDEX, crtc1A);
	}

	NVWriteRAMDAC(dev, 0, NV_PRAMDAC_PLL_COEFF_SELECT, state->pllsel);

	get_slave_funcs(encoder)->dpms(encoder, mode);
}

static void nv04_tv_bind(struct drm_device *dev, int head, bool bind)
{
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct nv04_crtc_reg *state = &dev_priv->mode_reg.crtc_reg[head];

	state->tv_setup = 0;

	if (bind)
		state->CRTC[NV_CIO_CRE_49] |= 0x10;
	else
		state->CRTC[NV_CIO_CRE_49] &= ~0x10;

	NVWriteVgaCrtc(dev, head, NV_CIO_CRE_LCD__INDEX,
		       state->CRTC[NV_CIO_CRE_LCD__INDEX]);
	NVWriteVgaCrtc(dev, head, NV_CIO_CRE_49,
		       state->CRTC[NV_CIO_CRE_49]);
	NVWriteRAMDAC(dev, head, NV_PRAMDAC_TV_SETUP,
		      state->tv_setup);
}

static void nv04_tv_prepare(struct drm_encoder *encoder)
{
	struct drm_device *dev = encoder->dev;
	int head = nouveau_crtc(encoder->crtc)->index;
	struct drm_encoder_helper_funcs *helper = encoder->helper_private;

	helper->dpms(encoder, DRM_MODE_DPMS_OFF);

	nv04_dfp_disable(dev, head);

	if (nv_two_heads(dev))
		nv04_tv_bind(dev, head ^ 1, false);

	nv04_tv_bind(dev, head, true);
}

static void nv04_tv_mode_set(struct drm_encoder *encoder,
			     struct drm_display_mode *mode,
			     struct drm_display_mode *adjusted_mode)
{
	struct drm_device *dev = encoder->dev;
	struct drm_nouveau_private *dev_priv = dev->dev_private;
	struct nouveau_crtc *nv_crtc = nouveau_crtc(encoder->crtc);
	struct nv04_crtc_reg *regp = &dev_priv->mode_reg.crtc_reg[nv_crtc->index];

	regp->tv_htotal = adjusted_mode->htotal;
	regp->tv_vtotal = adjusted_mode->vtotal;

	/* These delay the TV signals with respect to the VGA port,
	 * they might be useful if we ever allow a CRTC to drive
	 * multiple outputs.
	 */
	regp->tv_hskew = 1;
	regp->tv_hsync_delay = 1;
	regp->tv_hsync_delay2 = 64;
	regp->tv_vskew = 1;
	regp->tv_vsync_delay = 1;

	get_slave_funcs(encoder)->mode_set(encoder, mode, adjusted_mode);
}

static void nv04_tv_commit(struct drm_encoder *encoder)
{
	struct nouveau_encoder *nv_encoder = nouveau_encoder(encoder);
	struct drm_device *dev = encoder->dev;
	struct nouveau_crtc *nv_crtc = nouveau_crtc(encoder->crtc);
	struct drm_encoder_helper_funcs *helper = encoder->helper_private;

	helper->dpms(encoder, DRM_MODE_DPMS_ON);

	NV_INFO(dev, "Output %s is running on CRTC %d using output %c\n",
		      drm_get_connector_name(&nouveau_encoder_connector_get(nv_encoder)->base), nv_crtc->index,
		      '@' + ffs(nv_encoder->dcb->or));
}

static void nv04_tv_destroy(struct drm_encoder *encoder)
{
	get_slave_funcs(encoder)->destroy(encoder);
	drm_encoder_cleanup(encoder);

	kfree(encoder->helper_private);
	kfree(nouveau_encoder(encoder));
}

static const struct drm_encoder_funcs nv04_tv_funcs = {
	.destroy = nv04_tv_destroy,
};

int
nv04_tv_create(struct drm_connector *connector, struct dcb_entry *entry)
{
#if !defined(__AROS__)
	struct nouveau_encoder *nv_encoder;
	struct drm_encoder *encoder;
	struct drm_device *dev = connector->dev;
	struct drm_encoder_helper_funcs *hfuncs;
	struct drm_encoder_slave_funcs *sfuncs;
	struct nouveau_i2c_chan *i2c =
		nouveau_i2c_find(dev, entry->i2c_index);
	int type, ret;

	/* Ensure that we can talk to this encoder */
	type = nv04_tv_identify(dev, entry->i2c_index);
	if (type < 0)
		return type;

	/* Allocate the necessary memory */
	nv_encoder = kzalloc(sizeof(*nv_encoder), GFP_KERNEL);
	if (!nv_encoder)
		return -ENOMEM;

	hfuncs = kzalloc(sizeof(*hfuncs), GFP_KERNEL);
	if (!hfuncs) {
		ret = -ENOMEM;
		goto fail_free;
	}

	/* Initialize the common members */
	encoder = to_drm_encoder(nv_encoder);

	drm_encoder_init(dev, encoder, &nv04_tv_funcs, DRM_MODE_ENCODER_TVDAC);
	drm_encoder_helper_add(encoder, hfuncs);

	encoder->possible_crtcs = entry->heads;
	encoder->possible_clones = 0;
	nv_encoder->dcb = entry;
	nv_encoder->or = ffs(entry->or) - 1;

	/* Run the slave-specific initialization */
	ret = drm_i2c_encoder_init(dev, to_encoder_slave(encoder),
				   &i2c->adapter, &nv04_tv_encoder_info[type]);
	if (ret < 0)
		goto fail_cleanup;

	/* Fill the function pointers */
	sfuncs = get_slave_funcs(encoder);

	*hfuncs = (struct drm_encoder_helper_funcs) {
		.dpms = nv04_tv_dpms,
		.save = sfuncs->save,
		.restore = sfuncs->restore,
		.mode_fixup = sfuncs->mode_fixup,
		.prepare = nv04_tv_prepare,
		.commit = nv04_tv_commit,
		.mode_set = nv04_tv_mode_set,
		.detect = sfuncs->detect,
	};

	/* Attach it to the specified connector. */
	sfuncs->create_resources(encoder, connector);
	drm_mode_connector_attach_encoder(connector, encoder);

	return 0;

fail_cleanup:
	drm_encoder_cleanup(encoder);
	kfree(hfuncs);
fail_free:
	kfree(nv_encoder);
	return ret;
#else
    IMPLEMENT("\n");
    return 0;
#endif
}

