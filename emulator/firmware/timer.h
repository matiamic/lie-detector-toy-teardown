#ifndef TIMER_H
#define TIMER_H

class Timer
{
	public:
		Timer();
                void set_ms(int time_ms);
                bool finished();
                bool is_alive();

	private:
                long int _time_start;
                long int _time_set_ms;
                bool _is_alive = false;
};

#endif
