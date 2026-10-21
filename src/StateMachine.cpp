//#include <stack>
//
//

struct StateMachine;
struct State {
	virtual void update(StateMachine& controller) = 0;
	virtual void on_enter(StateMachine& controller) = 0;
	virtual void on_exit(StateMachine& controller) = 0;
};
struct StateMachine {

	State* m_State = nullptr;
	virtual ~StateMachine() = default;

	virtual void update() {
		m_State->update(*this);
	}


	virtual void change_state(State* newState) {
		// call this from State::Update()
		// exit if already has a state
		if (m_State) {
			m_State->on_exit(*this);
		}
		m_State = newState;
		m_State->on_enter(*this);
	}
};
//// example
//struct EnemyChase : public State {
//	Vec3 to_player;
//	virtual void update(StateMachine& enemy) {
//		EnemyAI& r = static_cast<EnemyAI&>(enemy);
//		EnemyEvent _event = r.m_EnemyEvents.top(); r.m_EnemyEvents.pop();
//		switch (_event) {
//		case EnemyEvent::READY_ENGAGE: {
//			enemy.change_state(&EnemyAI::m_EngagingPlayer);
//		}break;
//		default:	// keep patrolling - Patrol(Enemy), this would just do += 0.1 or w/e toward the target
//		}
//	};
//	virtual void on_enter(StateMachine& enemy) {
//		Vec3 to_player = enemy.worldPos.direction(player);
//		Move(enemy, to_player);
//	};
//	virtual void on_exit(StateMachine& enemy) {
//		// StopMoving(enemy);
//	};
//};
//struct EnemyEngage : public State {
//	// Inherited via State a
//	void update(StateMachine& controller) override
//	{}
//	void on_enter(StateMachine& controller) override
//	{}
//	void on_exit(StateMachine& controller) override
//	{}
//};
//struct EnemyAI : public StateMachine {
// this uses static states, so they can be shared, but cant have any individual data inside the states such as a "time since last seen" which would be needed in this case if >1 npc 
// in that case, use instantiated states, allocate on switch state, free on exit
//	static EnemyPatrol m_PatrolState;
//	static EnemyChase m_ChaseState;
//	static EnemyEngage m_EngagingPlayer;
//	std::stack<EnemyEvent> m_EnemyEvents;
//};
//
//struct EnemyPatrol : public State {
//	Vec3 target;
//	void update(StateMachine& enemy)
//	{
//		EnemyAI& r = static_cast<EnemyAI&>(enemy);
//		EnemyEvent _event = r.m_EnemyEvents.top(); r.m_EnemyEvents.pop();
//		switch (_event) {
//		case EnemyEvent::SPOTTED_PLAYER: {
//			enemy.change_state(&EnemyAI::m_ChaseState);
//		}break;
//		default:	// keep patrolling - Patrol(Enemy), this would just do += 0.1 or w/e toward the target
//		}
//	}
//	void on_enter(StateMachine& enemy)
//	{
//		// Begin Patrolling
//		// BeginPatrol(enemy);
//	}
//	void on_exit(StateMachine& enemy)
//	{
//		//StopMoving(enemy);
//		// check
//	}
//};


//int main() {
//	StateMachine controller;
//	controller.change_state(&static_state); // this will call the on_enter setup code.
//	controller.update(); // calls the state specific update function.
//}