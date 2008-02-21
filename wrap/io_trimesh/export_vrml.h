/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
History
$Log: not supported by cvs2svn $
Revision 1.7  2008/02/21 17:23:57  cignoni
Corrected various bug, involving spurious commas, and pervertex color saved as per wedge color.

Revision 1.6  2007/06/12 10:15:35  cignoni
Very important change. No more scaling and translation in the saved file!

Revision 1.5  2007/03/20 16:47:49  cignoni
Update to the new texture syntax

Revision 1.4  2006/11/21 19:22:53  e_cerisoli
Added Comments for documentation

****************************************************************************/

#ifndef __VCGLIB_EXPORT_WRL
#define __VCGLIB_EXPORT_WRL

#include <stdio.h>
#include <wrap/io_trimesh/io_mask.h>

#include "io_material.h"

namespace vcg {
	namespace tri {
		namespace io {

			template <class SaveMeshType>
			/** 
			This class encapsulate a filter for save vrml meshes.
			*/
			class ExporterWRL
			{
			public:
				typedef typename SaveMeshType::VertexPointer VertexPointer;
				typedef typename SaveMeshType::ScalarType ScalarType;
				typedef typename SaveMeshType::VertexType VertexType;
				typedef typename SaveMeshType::FaceType FaceType;
				typedef typename SaveMeshType::VertexIterator VertexIterator;
				typedef typename SaveMeshType::FaceIterator FaceIterator;
				
				///Standard call for saving a mesh
				static int Save(SaveMeshType &m, const char * filename, const int &mask, CallBackPos *cb=0)
				{					
					FILE *fp;
					fp = fopen(filename,"wb");
					if(fp==NULL)
						return 1;

					// Header					
					fprintf(fp,
						 "#VRML V2.0 utf8\n"  
   	 					"\n" 	 					
						"# Generated by VCGLIB, (C)Copyright 1999-2001 VCG, IEI-CNR\n"
						"\n"						
						"NavigationInfo {\n"						
						"	type [ \"EXAMINE\", \"ANY\" ]\n"						
						"}\n"						
						);					

					// Tranche principale
					double ss = 8.0/m.bbox.Diag();					
					
					fprintf(fp,
					"Transform {\n"
					"  scale %g %g %g\n"
					"  translation %g %g %g\n"
					"  children\n"
					"  [\n"
					,1.0,1.0,1.0,
					 0.0,0.0,0.0);
//					,ss,ss,ss
//					,  -m.bbox.Center()[0]*ss,  -m.bbox.Center()[1]*ss,-3-m.bbox.Center()[2]*ss	);

					// Start Shape
					fprintf(fp,
					"    Shape\n"
					"    {\n"
					"      geometry IndexedFaceSet\n"
					"      {\n"
					"        creaseAngle .5\n"
					"        solid FALSE\n"
					"        coord Coordinate\n"
					"        {\n"
					"          point\n"
					"          ["
					);
					FaceIterator fi;	
					VertexIterator vi;	
					std::map<VertexPointer,int> index;
					int ind;

					// Vertici
					for(ind=0,vi=m.vert.begin(); vi!=m.vert.end(); ++vi, ++ind)	
					if(!(*vi).IsD())
						{
							if(vi!=m.vert.begin()) fprintf(fp,", ");
							if(ind%4==0) fprintf(fp, "\n            " );
							fprintf(fp, "%g %g %g" ,(*vi).P()[0] ,(*vi).P()[1] ,(*vi).P()[2] 	);
							index[&*vi] = ind;
						}
					fprintf(fp,"\n"
								"          ]\n"
								"        }\n"
								);
					
					
					if( HasPerVertexColor(m) && (mask & vcg::tri::io::Mask::IOM_VERTCOLOR))
					{
						fprintf(fp,
							"        color Color\n"
							"        {\n"
							"          color\n"
							"          ["
							);
						for(ind=0,vi=m.vert.begin();vi!=m.vert.end();++vi,++ind)
						if(!(*vi).IsD())
						{
							if(vi!=m.vert.begin()) fprintf(fp,", ");
							float r = float(vi->C()[0])/255;
							float g = float(vi->C()[1])/255;
							float b = float(vi->C()[2])/255;

							if(ind%4==0)
								fprintf(fp,"\n            ");
							fprintf(fp,"%g %g %g",r,g,b);
						}
						fprintf(fp,	
							"\n"
							"          ]\n"							
							"        }\n"
							);
					}
					else if( HasPerWedgeColor(m) &&  (mask & vcg::tri::io::Mask::IOM_WEDGCOLOR ))
					{						
						fprintf(fp,
							"        color Color\n"	
							"        {\n"							
							"          color\n"							
							"          ["							);
						for(ind=0,fi=m.face.begin();fi!=m.face.end();++fi,++ind)
						if(!(*fi).IsD())
						{
							if(fi!=m.face.begin()) fprintf(fp,", ");
							if(ind%4==0) fprintf(fp,"\n            ");
								for(int z=0;z<3;++z)							
								{								
									if(z!=0) fprintf(fp,", ");
									float r = float(fi->WC(z)[0])/255;
									float g = float(fi->WC(z)[1])/255;
									float b = float(fi->WC(z)[2])/255;
									fprintf(fp,"%g %g %g",r,g,b);
								}
						}						
						fprintf(fp,							
							"\n"							
							"          ]\n"							
							"        }\n"						
							"        colorIndex\n"							
							"        ["							);						
						int nn = 0;						
						for(ind=0,fi=m.face.begin(); fi!=m.face.end(); ++fi,++ind)
						if(!(*fi).IsD())
						{
							//if(fi!=m.face.begin()) fprintf(fp,", ");
							if(ind%4==0) fprintf(fp,"\n          ");							
									fprintf(fp,"%i",nn++);								
									fprintf(fp,"%i",nn++);								
									fprintf(fp,"%i",nn++);								
							fprintf(fp,"-1");						
						}						
						fprintf(fp,							
							"\n"							
							"       ]\n"							
							);					
					}
					// NOTE MULTITEXTURE WRL SAVING DO NOT WORK!
					else if (HasPerWedgeTexCoord(m) &&(mask &  vcg::tri::io::Mask::IOM_WEDGTEXCOORD))
					{
						fprintf(fp,
							"\n"
							"        texCoord TextureCoordinate\n"
							"        {\n"
							"          point\n"
							"          [\n"
							);
						for(ind=0,fi=m.face.begin(); fi!=m.face.end(); ++fi,++ind)
						if(!(*fi).IsD())
							{
								//if(fi!=m.face.begin()) fprintf(fp,", ");
								if(ind%4==0) fprintf(fp,"\n          ");
								for (int j = 0; j < 3; j++)
										fprintf(fp,"%g %g ",fi->WT(j).u(),fi->WT(j).v());
							}
						fprintf(fp,
							"\n"
							"          ]\n"
							"        }\n"
							"        texCoordIndex\n"
							"        [\n"
							);
						int nn = 0;
						for(ind=0,fi=m.face.begin(); fi!=m.face.end(); ++fi,++ind)
							if(!(*fi).IsD())
							{
								//if(fi!=m.face.begin()) fprintf(fp,", ");
								if(ind%4==0) fprintf(fp,"\n          ");								
								for (int j = 0; j < 3; j++)
									fprintf(fp,"%d ",nn++);
								fprintf(fp,"-1 ");
							}
						fprintf(fp,
							"\n"
							"        ]\n"
							);
					}
					fprintf(fp,
							"        coordIndex\n"
							"        ["
							);
					// Facce
					for(ind=0,fi=m.face.begin(); fi!=m.face.end(); ++fi,++ind)
					if(!(*fi).IsD())
					{
						if(fi!=m.face.begin()) fprintf(fp,", ");
						if(ind%6==0) fprintf(fp,"\n          ");								
						for (int j = 0; j < 3; j++)
							fprintf(fp,"%i,",index[(*fi).V(j)]);
						
						fprintf(fp,"-1");	
					}
					fprintf(fp,
							"\n"
							"        ]\n"
							"      }\n"
							"      appearance Appearance\n"
							"      {\n"						
							"        material Material\n"
							"        {\n"
							"	       ambientIntensity 0.2\n"
							"	       diffuseColor 0.9 0.9 0.9\n"
							"	       specularColor .1 .1 .1\n"
							"	       shininess .5\n"
							"        }\n"
							);
					if(m.textures.size())
					{
						fprintf(fp,
							"        texture ImageTexture { url \"  %s  \" }\n"
							,m.textures[0].c_str()
							);
					}
					fprintf(fp,
						"      }\n"
						"    }\n"
						"  ]\n"
						"}\n"
						);
					fclose(fp);
					return 0;
	}
	///Returns mask of capability one define with what are the saveable information of the format.
	static int GetExportMaskCapability()
	{
		int capability = 0;

		//vert
		capability |= MeshModel::IOM_VERTCOLOR;
		
		//wedg
		capability |= MeshModel::IOM_WEDGTEXCOORD;
		capability |= MeshModel::IOM_WEDGCOLOR;

		return capability;
	}

	/// Standard call for knowing the meaning of an error code
	static const char *ErrorMsg(int error)
	{
		static std::vector<std::string> wrl_error_msg;
		if(wrl_error_msg.empty())
		{
			wrl_error_msg.resize(2 );
			wrl_error_msg[0]="No errors";
			wrl_error_msg[1]="Can't open file";
		}
		if(error>1 || error<0) return "Unknown error";
		else return wrl_error_msg[error].c_str();
		}

}; // end class
} // end Namespace io
} // end Namespace tri
} // end Namespace vcg

#endif