		uint32 c;
		paletteRam+=(idx&0x01)*256*2;
		if (flags&FLAGS_READMODIFY)
		{
			if (flags&FLAGS_HFLIP)
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
							*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
						}
						else
							current_line_buffer-=2;
					}
					else
					{
						*current_line_buffer--=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
						*current_line_buffer--=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
					}
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
							*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
						}
						else
							current_line_buffer+=2;
					}
					else
					{
						*current_line_buffer++=BLEND_Y(*current_line_buffer,paletteRam[c+0]);
						*current_line_buffer++=BLEND_CC(*current_line_buffer,paletteRam[c+1]);
					}
					iwidth--;
				}
			}
		}
		else
		{
			if (flags&FLAGS_HFLIP)
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer--=paletteRam[c+0];
							*current_line_buffer--=paletteRam[c+1];
						}
						else
							current_line_buffer-=2;
					}
					else
					{
						*current_line_buffer--=paletteRam[c+0];
						*current_line_buffer--=paletteRam[c+1];
					}
					iwidth--;
				}
			}
			else
			{
				while (iwidth)
				{
					c=jaguar_byte_read(ptr++);
					c<<=1;
					if (flags&FLAGS_TRANSPARENT)
					{					
						if (c)
						{
							*current_line_buffer++=paletteRam[c+0];
							*current_line_buffer++=paletteRam[c+1];
						}
						else
							current_line_buffer+=2;
					}
					else
					{
						*current_line_buffer++=paletteRam[c+0];
						*current_line_buffer++=paletteRam[c+1];
					}
					iwidth--;
				}
			}
		}