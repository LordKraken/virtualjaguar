		uint8	*paletteRam=&tom_ram_8[0x400];
		paletteRam+=idx*4;
		
		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						if (c)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[((c&0x0f)<<1)+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[((c&0x0f)<<1)+1]);
						}
						else
							current_line_buffer-=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[((c&0x0f)<<1)+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[((c&0x0f)<<1)+1]);

						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
			else
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						if (c)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[((c&0x0f)<<1)+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[((c&0x0f)<<1)+1]);
						}
						else
							current_line_buffer+=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[((c&0x0f)<<1)+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[((c&0x0f)<<1)+1]);

						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
		}
		else
		{
			if (flags&FLAGS_HFLIP)
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						if (c)
						{
							*current_line_buffer--=paletteRam[((c&0x0f)<<1)+0];
							*current_line_buffer--=paletteRam[((c&0x0f)<<1)+1];
						}
						else
							current_line_buffer-=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						*current_line_buffer--=paletteRam[((c&0x0f)<<1)+0];
						*current_line_buffer--=paletteRam[((c&0x0f)<<1)+1];

						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
			else
			{
				if (flags&FLAGS_TRANSPARENT)
				{					
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						if (c)
						{
							*current_line_buffer++=paletteRam[((c&0x0f)<<1)+0];
							*current_line_buffer++=paletteRam[((c&0x0f)<<1)+1];
						}
						else
							current_line_buffer+=2;
						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
				else
				{
					while (scaled_width)
					{
						uint32 c=jaguar_byte_read(ptr+(((cnt>>16) & (~0x0f))*pitch)+((cnt>>16)&0x0f));
						if (!(cnt&(1<<16)))
							c>>=4;
						*current_line_buffer++=paletteRam[((c&0x0f)<<1)+0];
						*current_line_buffer++=paletteRam[((c&0x0f)<<1)+1];

						cnt+=hscale_fixed;
						scaled_width--;
					}
				}
			}
		}
