#ifndef _STONEBUILDINGOBJECT_H_
#define _STONEBUILDINGOBJECT_H_

#include "RenderObject.h"
#include "CommonRenderObjects.h"

class StoneBuildingObject:public StoneBuildingBase
{
public:
	virtual void Build(const D3DApp* pApp)override;
	virtual void Draw(const D3DApp* pApp)override;
};

class ReflectSkullObject:public BasicSkullObject
{
public:

	virtual void Build(const D3DApp* pApp)override;
	virtual void Draw(const D3DApp* pApp)override;

private:

};
#endif