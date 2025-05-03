#ifndef _Eon_Player_h_
#define _Eon_Player_h_

namespace Ecs {


class PlayerBodyComponent;
using PlayerBodyComponentPtr = Ptr<PlayerBodyComponent>;


typedef enum : byte {
	Unspecified,
	Left,
	Right
} PlayerHandedness;

typedef enum : byte {
	High,
	Approximate,
} HandPositionAccuracy;

struct HandActionSourcePose {
	vec3 forward_direction;
	quat orientation;
	vec3 position;
	HandPositionAccuracy position_accuracy;
	vec3 up_direction;
	
	vec3 GetPosition() const {return position;}
	vec3 GetForwardDirection() const {return forward_direction;}
	quat GetOrientation() const {return orientation;}
};

// "SpatialSource"
struct HandLocationSource : ControllerSource {
	
};

struct HandSourceLocation {
	vec3 position;
	quat orientation;
	HandActionSourcePose* pose = 0;
	
	const vec3& GetPosition() const {return position;}
	const quat& GetOrientation() const {return orientation;}
	virtual HandActionSourcePose* GetHandPose() const {return pose;}
	
};

struct HandActionSourceLocation : HandSourceLocation {
	
	
};


class PlayerHandComponent : public Component<PlayerHandComponent> {
	
public:
	COMP_DEF_VISIT_(vis & body)
	
	COPY_PANIC(PlayerHandComponent)
	
	void Serialize(Stream& e) override;
	void Initialize() override;
	void Uninitialize() override;
	bool Arg(String key, Value value) override;
	
    bool IsSource(const ControllerSource& rhs) const;
	
	
	PlayerBodyComponentPtr				body;
    const HandLocationSource*			source = 0;
    HandActionSourceLocation*			location = 0;
	
	bool								is_simulated = false;
    bool								attach_ctrl_model = false;
    byte								req_hand = Unspecified;
    
};

using PlayerHandComponentPtr = Ptr<PlayerHandComponent>;



class PlayerHeadComponent : public Component<PlayerHeadComponent> {
	
public:
	COMP_DEF_VISIT_(vis & body)
	
	COPY_PANIC(PlayerHeadComponent)
	
	void Serialize(Stream& e) override;
	void Initialize() override;
	void Uninitialize() override;
	bool Arg(String key, Value value) override;
	
	
	PlayerBodyComponentPtr				body;
	
};

using PlayerHeadComponentPtr = Ptr<PlayerHeadComponent>;



class PlayerBodyComponent : public Component<PlayerBodyComponent> {
	
protected:
	friend class PlayerBodySystem;
	
	PlayerHandComponentPtr hands[2];
	PlayerHeadComponentPtr head;
	
	float height = 1.8f;
	
public:
	COMP_DEF_VISIT_(vis & hands[0] & hands[1] & head)
	COPY_PANIC(PlayerBodyComponent)
	
	void Serialize(Stream& e) override;
	void Initialize() override;
	void Uninitialize() override;
	bool Arg(String key, Value value) override;
	
	bool SetHand(PlayerHandedness hand, PlayerHandComponentPtr comp);
	bool SetHead(PlayerHeadComponentPtr head);
	
	float GetHeight() const {return height;}
	const PlayerHeadComponentPtr& GetHead() const {return head;}
	
};




class PlayerBodySystem :
    public System<PlayerBodySystem>,
    public InteractionListener
{
	
public:
	ECS_SYS_CTOR(PlayerBodySystem)
	ECS_SYS_DEF_VISIT_(vis && bodies)
	
	const Array<PlayerBodyComponentPtr>& GetComponents() const {return bodies;}
	
	void Attach(PlayerBodyComponentPtr h);
	void Detach(PlayerBodyComponentPtr h);
protected:
    // System
    bool Initialize() override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
	
	void OnControllerDetected(const CtrlEvent& e) override;
	void OnControllerLost(const CtrlEvent& e) override;
	void OnControllerPressed(const CtrlEvent& e) override;
	void OnControllerUpdated(const CtrlEvent& e) override;
	void OnControllerReleased(const CtrlEvent& e) override;
    
private:
    void RefreshComponentsForSource(const HandLocationSource& source);
    
    Ptr<InteractionSystem> iasys;
    Array<PlayerBodyComponentPtr> bodies;
    
};



}

#endif
