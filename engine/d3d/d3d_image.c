#include "quakedef.h"
#include "winquake.h"
#ifdef D3D9QUAKE
#if !defined(HMONITOR_DECLARED) && (WINVER < 0x0500)
    #define HMONITOR_DECLARED
    DECLARE_HANDLE(HMONITOR);
#endif
#include <d3d9.h>
extern LPDIRECT3DDEVICE9 pD3DDev9;

void D3D9_DestroyTexture (texid_t tex)
{
	if (!tex)
		return;

	if (tex->ptr)
		IDirect3DBaseTexture9_Release((IDirect3DBaseTexture9*)tex->ptr);
	tex->ptr = NULL;
}

qboolean D3D9_LoadTextureMips(image_t *tex, const struct pendingtextureinfo *mips)
{
	qbyte *fte_restrict out, *fte_restrict in;
	int x, y, i;
	D3DLOCKED_RECT lock;
	D3DFORMAT fmt;
	D3DSURFACE_DESC desc;
	IDirect3DBaseTexture9 *dbt;
	qboolean swap = false;
	unsigned int blockwidth, blockheight, blockbytes;

	switch(mips->encoding)
	{
	case PTI_RGB565:
		fmt = D3DFMT_R5G6B5;
		break;
	case PTI_RGBA4444://not supported on d3d9
		return false;
	case PTI_ARGB4444:
		fmt = D3DFMT_A4R4G4B4;
		break;
	case PTI_RGBA5551://not supported on d3d9
		return false;
	case PTI_ARGB1555:
		fmt = D3DFMT_A1R5G5B5;
		break;
	case PTI_RGBA8_SRGB:
	case PTI_RGBA8:
//		fmt = D3DFMT_A8B8G8R8;	/*how do we check 
		fmt = D3DFMT_A8R8G8B8;
		swap = true;
		break;
	case PTI_RGBX8_SRGB:
	case PTI_RGBX8:
//		fmt = D3DFMT_X8B8G8R8;
		fmt = D3DFMT_X8R8G8B8;
		swap = true;
		break;
	case PTI_BGRA8_SRGB:
	case PTI_BGRA8:
		fmt = D3DFMT_A8R8G8B8;
		break;
	case PTI_BGRX8_SRGB:
	case PTI_BGRX8:
		fmt = D3DFMT_X8R8G8B8;
		break;

	//too lazy to support these for now
	case PTI_BC1_RGB_SRGB:
	case PTI_BC1_RGBA_SRGB:	//d3d doesn't distinguish between these
	case PTI_BC1_RGB:
	case PTI_BC1_RGBA:	//d3d doesn't distinguish between these
		fmt = D3DFMT_DXT1;
		break;
	case PTI_BC2_RGBA_SRGB:
	case PTI_BC2_RGBA:
		fmt = D3DFMT_DXT3;
		break;
	case PTI_BC3_RGBA_SRGB:
	case PTI_BC3_RGBA:
		fmt = D3DFMT_DXT5;
		break;

	//bc4-7 not supported on d3d9.
	//etc2 have no chance.

	default:	//no idea
		return false;
	}

	Image_BlockSizeForEncoding(mips->encoding, &blockbytes, &blockwidth, &blockheight);

	if (!pD3DDev9)
		return false;	//can happen on errors
	if (mips->type == PTI_CUBEMAP)
	{
		IDirect3DCubeTexture9 *dt;
		if (FAILED(IDirect3DDevice9_CreateCubeTexture(pD3DDev9, mips->mip[0].width, mips->mipcount/6, 0, fmt, D3DPOOL_MANAGED, &dt, NULL)))
			return false;
		dbt = (IDirect3DBaseTexture9*)dt;

		for (i = 0; i < mips->mipcount; i++)
		{
			IDirect3DCubeTexture9_GetLevelDesc(dt, i/6, &desc);

			if (mips->mip[i].height != desc.Height || mips->mip[i].width != desc.Width)
			{
				IDirect3DCubeTexture9_Release(dt);
				return false;
			}

			IDirect3DCubeTexture9_LockRect(dt, i%6, i/6, &lock, NULL, D3DLOCK_NOSYSLOCK|D3DLOCK_DISCARD);
			//can't do it in one go. pitch might contain padding or be upside down.
			if (!mips->mip[i].data)
				;
			else if (swap)
			{	//only works for blockbytes=4
				size_t rowbytes = ((mips->mip[i].width+blockwidth-1)/blockwidth)*blockbytes;
				for (y = 0, out = lock.pBits, in = mips->mip[i].data; y < mips->mip[i].height; y+=blockheight, out += lock.Pitch, in += rowbytes)
				{
					for (x = 0; x < rowbytes; x+=4)
					{
						out[x+0] = in[x+2];
						out[x+1] = in[x+1];
						out[x+2] = in[x+0];
						out[x+3] = in[x+3];
					}
				}
			}
			else
			{
				size_t rowbytes = ((mips->mip[i].width+blockwidth-1)/blockwidth)*blockbytes;
				for (y = 0, out = lock.pBits, in = mips->mip[i].data; y < mips->mip[i].height; y+=blockheight, out += lock.Pitch, in += rowbytes)
					memcpy(out, in, rowbytes);
			}
			IDirect3DCubeTexture9_UnlockRect(dt, i%6, i/6);
		}
	}
	else
	{
		IDirect3DTexture9 *dt;
		if (FAILED(IDirect3DDevice9_CreateTexture(pD3DDev9, mips->mip[0].width, mips->mip[0].height, mips->mipcount, 0, fmt, D3DPOOL_MANAGED, &dt, NULL)))
			return false;
		dbt = (IDirect3DBaseTexture9*)dt;
	
		for (i = 0; i < mips->mipcount; i++)
		{
			IDirect3DTexture9_GetLevelDesc(dt, i, &desc);

			if (mips->mip[i].height != desc.Height || mips->mip[i].width != desc.Width)
			{
				IDirect3DTexture9_Release(dt);
				return false;
			}

			IDirect3DTexture9_LockRect(dt, i, &lock, NULL, D3DLOCK_NOSYSLOCK|D3DLOCK_DISCARD);
			//can't do it in one go. pitch might contain padding or be upside down.
			if (!mips->mip[i].data)
				;
			else if (swap)
			{
				size_t rowbytes = ((mips->mip[i].width+blockwidth-1)/blockwidth)*blockbytes;
				for (y = 0, out = lock.pBits, in = mips->mip[i].data; y < mips->mip[i].height; y+=blockheight, out += lock.Pitch, in += rowbytes)
				{
					for (x = 0; x < rowbytes; x+=4)
					{
						out[x+0] = in[x+2];
						out[x+1] = in[x+1];
						out[x+2] = in[x+0];
						out[x+3] = in[x+3];
					}
				}
			}
			else
			{
				size_t rowbytes = ((mips->mip[i].width+blockwidth-1)/blockwidth)*blockbytes;
				for (y = 0, out = lock.pBits, in = mips->mip[i].data; y < mips->mip[i].height; y+=blockheight, out += lock.Pitch, in += rowbytes)
					memcpy(out, in, rowbytes);
			}
			IDirect3DTexture9_UnlockRect(dt, i);
		}
	}
	D3D9_DestroyTexture(tex);
	tex->ptr = dbt;

	return true;
}
void D3D9_UploadLightmap(lightmapinfo_t *lm)
{
}

#endif
