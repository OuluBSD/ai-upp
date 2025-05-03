#pragma once

#if 1

namespace Ecs {


class PaintStrokeComponent :
	public Component<PaintStrokeComponent> {
	
public:
	RTTI_COMP0(PaintStrokeComponent)
	COPY_PANIC(PaintStrokeComponent)
	COMP_DEF_VISIT
	
	
	Vector<Square> squares;
	bool stroke_changed = false;
	
	
	void Serialize(Stream& e) override;
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
