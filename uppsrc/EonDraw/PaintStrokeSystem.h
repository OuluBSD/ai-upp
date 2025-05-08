#pragma once

#if 1

namespace Ecs {


class PaintStrokeComponent :
	public Component<PaintStrokeComponent> {
	
public:
	Vector<Square> squares;
	bool stroke_changed = false;
	
	
	void Visit(Vis& v) override;
	void Initialize() override;
	void Uninitialize() override;
	void AddPoint(const mat4& trans_mtx, float width);
	
};


class PaintStrokeSystemBase :
	public System<PaintStrokeSystemBase>
{
public:
	ECS_SYS_CTOR(PaintStrokeSystemBase);
	ECS_SYS_DEF_VISIT
	
	~PaintStrokeSystemBase() = default;
	
	void Attach(PaintStrokeComponent* comp);
	void Detach(PaintStrokeComponent* comp);
	
protected:
	void Update(double) override;
	bool Initialize() override;
	
private:
	Vector<PaintStrokeComponent*> comps;
	
};


}

#endif
