    /*

    Copyright (C) 2000-2002 Stefan Westerfeld
                            stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "config.h"

#include "virtualports.h"
#include "startupmanager.h"
#include "gslschedule.h"
#include "debug.h"
#include "asyncschedule.h"
#include "audiosubsys.h"
#include <gsl/gslcommon.h>
#include <gsl/gslengine.h>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <stack>

/* HACK */
class GslMainLoop {
protected:
	std::list<GslClass *> freeClassList;

public:
	GslEngineLoop loop;

	static bool waitOnTransNeedData;
	static bool gslDataCalculated;

	/* static check function */
	static gboolean gslCheck(gpointer /* data */, guint /* n_values */,
							 glong*  /* timeout_p */,
                         	 guint /* n_fds */, const GPollFD* /* fds */,
						 	 gboolean /* revents_filled */)
	{
		return waitOnTransNeedData;
	}
	/* mainloop integration: initialize (called to get initial loop setup) */
	void initialize()
	{
		gsl_transact(gsl_job_add_poll (gslCheck, 0, 0, 0, 0), 0);
		gsl_engine_prepare(&loop);

		for(unsigned int i = 0; i != loop.n_fds; i++)
		{
			printf("TODO: engine fd %d\n",i);
		}
	}
	/* mainloop integration: process (TODO - should be called by IOManager) */
	void process()
	{
		printf("TODO: mainloop wrapper for fd watches\n");
		if(gsl_engine_check(&loop))
			gsl_engine_dispatch();
	}
	/* wait for a transaction */
	void waitOnTrans()
	{
		arts_return_if_fail(waitOnTransNeedData == false);
		gsl_engine_wait_on_trans();
	}
	/* make the engine calculate something */
	void run()
	{
		waitOnTransNeedData = true;
		gslDataCalculated = false;

		while(!gslDataCalculated && gsl_engine_check(&loop))
			gsl_engine_dispatch();

		gslDataCalculated = false;
		waitOnTransNeedData = false;

		if(!freeClassList.empty())
		{
			/*
			 * make sure that all transactions that are still pending
			 * get finished (especially important in threaded case,
			 * since an entry in the free list doesn't necessarily
			 * mean that the module has entierly been freed)
			 */
			waitOnTrans();

			std::list<GslClass *>::iterator fi;
			for(fi = freeClassList.begin(); fi != freeClassList.end(); fi++)
				free(*fi);

			freeClassList.clear();
		}
	}
	void freeGslClass(GslClass *klass)
	{
		freeClassList.push_back(klass);
	}
} gslMainLoop;

bool GslMainLoop::waitOnTransNeedData = false;
bool GslMainLoop::gslDataCalculated = false;
namespace Arts { extern void *gslGlobalMutexTable; }


using namespace std;
using namespace Arts;

// ----------- Port -----------

Port::Port(const string& name, void *ptr, long flags, StdScheduleNode* parent)
	: _name(name), _ptr(ptr), _flags((AttributeType)flags),
	  parent(parent), _dynamicPort(false)
{
	_vport = new VPort(this);
}

Port::~Port()
{
	if(_vport)
		delete _vport;
}

AttributeType Port::flags()
{
	return _flags;
}

string Port::name()
{
	return _name;
}

ASyncPort *Port::asyncPort()
{
	return 0;
}

AudioPort *Port::audioPort()
{
	return 0;
}

void Port::addAutoDisconnect(Port *source)
{
	autoDisconnect.push_back(source);
	source->autoDisconnect.push_back(this);
}

void Port::removeAutoDisconnect(Port *source)
{
	std::list<Port *>::iterator adi;

	// remove our autodisconnection entry for source port
	adi = find(autoDisconnect.begin(),autoDisconnect.end(),source);
	assert(adi != autoDisconnect.end());
	autoDisconnect.erase(adi);

	// remove the source port autodisconnection entry to us
	adi=find(source->autoDisconnect.begin(),source->autoDisconnect.end(),this);
	assert(adi != source->autoDisconnect.end());
	source->autoDisconnect.erase(adi);
}

void Port::disconnectAll()
{
	if(_vport)
		delete _vport;
	_vport = 0;
	assert(autoDisconnect.empty());
	while(!autoDisconnect.empty())
	{
		Port *other = *autoDisconnect.begin();

		// syntax is disconnect(source)
		if(_flags & streamIn)
			// if we're incoming, other port is source
			vport()->disconnect(other->vport());
		else
			// if we're outgoing, we're the source
			other->vport()->disconnect(this->vport());
	}
}

void Port::setPtr(void *ptr)
{
	_ptr = ptr;
}

// ------- AudioPort ---------

AudioPort::AudioPort(const string& name,
		             void *ptr, long flags,StdScheduleNode *parent)
		: Port(name,ptr,flags,parent)
{
	destcount = 0;
	sourcemodule = 0;
	source = 0;
	gslIsConstant = false;
}

AudioPort::~AudioPort()
{
	//
}

AudioPort *AudioPort::audioPort()
{
	return this;
}

void AudioPort::setFloatValue(float f)
{
	gslIsConstant = true;
	gslConstantValue = f;

	parent->_connectionCountChanged = true;
}

void AudioPort::connect(Port *psource)
{
	if (source) return; // Error, should not happen (See BR70028)
	source = psource->audioPort();
	assert(source);
	addAutoDisconnect(psource);

	source->parent->_connectionCountChanged = parent->_connectionCountChanged = true;
	source->destcount++;
	sourcemodule = source->parent;

	// GSL connect
	GslTrans *trans = gsl_trans_open();
	gsl_trans_add(trans, gsl_job_connect(source->parent->gslModule,
										 source->gslEngineChannel,
										 parent->gslModule,
										 gslEngineChannel));
	gsl_trans_commit(trans);
}

void AudioPort::disconnect(Port *psource)
{
	if (!source || source != psource->audioPort()) return; // Error, should not happen (See BR70028)
	assert(source);
	assert(source == psource->audioPort());
	removeAutoDisconnect(psource);

	assert(sourcemodule == source->parent);
	sourcemodule = 0;

	source->parent->_connectionCountChanged = parent->_connectionCountChanged = true;
	source->destcount--;
	source = 0;

	// GSL disconnect
	GslTrans *trans = gsl_trans_open();
	gsl_trans_add(trans, gsl_job_disconnect(parent->gslModule,
										    gslEngineChannel));
	gsl_trans_commit(trans);
}

// --------- MultiPort ----------

MultiPort::MultiPort(const string& name,
		             void *ptr, long flags,StdScheduleNode *parent)
	: Port(name,ptr,flags,parent)
{
	conns = 0;
	nextID = 0;
	initConns();
}

MultiPort::~MultiPort()
{
	if(conns)
	{
		delete[] conns;
		conns = 0;
	}
}

void MultiPort::initConns()
{
	if(conns != 0) delete[] conns;
	conns = new float_ptr[parts.size() + 1];
	conns[parts.size()] = (float *)0;

	*(float ***)_ptr = conns;

	long n = 0;
	std::list<Part>::iterator i;
	for(i = parts.begin();i != parts.end(); i++)
	{
		AudioPort *p = i->dest;
		p->setPtr((void *)&conns[n++]);
	}
}

void MultiPort::connect(Port *port)
{
	AudioPort *dport;
	char sid[20];
	sprintf(sid,"%ld",nextID++);

	addAutoDisconnect(port);

	dport = new AudioPort("_"+_name+string(sid),0,streamIn,parent);

	Part part;
	part.src = (AudioPort *)port;
	part.dest = dport;

	parts.push_back(part);
	initConns();

	parent->addDynamicPort(dport);
	dport->vport()->connect(port->vport());
}

void MultiPort::disconnect(Port *sport)
{
	AudioPort *port = (AudioPort *)sport;
	removeAutoDisconnect(sport);

	std::list<Part>::iterator i;
	for(i = parts.begin(); i != parts.end(); i++)
	{
		if(i->src == port)
		{
			AudioPort *dport = i->dest;
			parts.erase(i);
			initConns();

			dport->vport()->disconnect(port->vport());
			parent->removeDynamicPort(dport);

			delete dport;
			return;
		}
	}
}

// -------- StdScheduleNode ---------

void StdScheduleNode::freeConn()
{
	if(inConn)
	{
		delete[] inConn;
		inConn = 0;
	}
	if(outConn)
	{
		delete[] outConn;
		outConn = 0;
	}
	inConnCount = outConnCount = 0;

	if(gslModule)
	{
		gsl_transact(gsl_job_discard(gslModule),0);

		gslModule = 0;
		gslRunning = false;
	}
}

void StdScheduleNode::gslProcess(GslModule *module, guint n_values)
{
	StdScheduleNode *node = (StdScheduleNode *)module->user_data;
	if(!node->running)		/* FIXME: need reasonable suspend in the engine */
		return;

	arts_return_if_fail(node->module != 0);

	GslMainLoop::gslDataCalculated = true;

	unsigned long j;
	for(j=0;j<node->inConnCount;j++)
	{
		if(node->inConn[j]->gslIsConstant)
			*((float **)node->inConn[j]->_ptr) =
				gsl_engine_const_values(node->inConn[j]->gslConstantValue);
		else
			*((float **)node->inConn[j]->_ptr) = const_cast<float *>(module->istreams[j].values);
	}

	for(j=0;j<node->outConnCount;j++)
		*((float **)node->outConn[j]->_ptr) = module->ostreams[j].values;

	node->module->calculateBlock(n_values);
}

static void gslModuleFree(gpointer /* data */, const GslClass *klass)
{
	gslMainLoop.freeGslClass(const_cast<GslClass *>(klass));
}

void StdScheduleNode::rebuildConn()
{
	std::list<Port *>::iterator i;

	freeConn();

	inConnCount = outConnCount = 0;
	inConn = new AudioPort_ptr[ports.size()];
	outConn = new AudioPort_ptr[ports.size()];

	for(i=ports.begin();i != ports.end();i++)
	{
		AudioPort *p = (*i)->audioPort();
		if(p)
		{
			if(p->flags() & streamIn)
			{
				p->gslEngineChannel = inConnCount;
				inConn[inConnCount++] = p;
			}
			if(p->flags() & streamOut)
			{
				p->gslEngineChannel = outConnCount;
				outConn[outConnCount++] = p;
			}
		}
	}

	/* create GSL node */
	GslClass *gslClass = (GslClass *)calloc(sizeof(GslClass),1);
	gslClass->n_istreams = inConnCount;
	gslClass->n_ostreams = outConnCount;
	gslClass->process = gslProcess;
	gslClass->free = gslModuleFree;

	gslModule = gsl_module_new (gslClass, (StdScheduleNode *)this);

	GslTrans *trans = gsl_trans_open();
	gsl_trans_add(trans,gsl_job_integrate(gslModule));
	gsl_trans_add(trans,gsl_job_set_consumer(gslModule, running));
	gslRunning = running;

	/* since destroying the old module and creating a new one will destroy
	 * all the connections, we need to restore them here
	 */
	unsigned int c;
	for(c = 0; c < inConnCount; c++)
	{
		if(inConn[c]->source)
		{
			gsl_trans_add(trans,
				gsl_job_connect(inConn[c]->source->parent->gslModule,
						 	 	inConn[c]->source->gslEngineChannel,
								inConn[c]->parent->gslModule,
								inConn[c]->gslEngineChannel));
		}
	}
	for(c = 0; c < outConnCount; c++)
	{
		std::list<Port *>::iterator ci;

		for(ci = outConn[c]->autoDisconnect.begin();
			ci != outConn[c]->autoDisconnect.end(); ci++)
		{
			AudioPort *dest = (*ci)->audioPort();
			if( dest )
			{
				gsl_trans_add(trans,
					gsl_job_connect(outConn[c]->parent->gslModule,
									outConn[c]->gslEngineChannel,
									dest->parent->gslModule,
									dest->gslEngineChannel));
			}
			else
			{
				arts_debug( "no audio port: %s for %s", ( *ci )->name().c_str(), _object->_interfaceName().c_str() );
			}
		}
	}
	gsl_trans_commit(trans);
}

Object_skel *StdScheduleNode::object()
{
	return _object;
}

void *StdScheduleNode::cast(const string &target)
{
	if(target == "StdScheduleNode") return (StdScheduleNode *)this;
	return 0;
}


void StdScheduleNode::accessModule()
{
	if(module) return;

	module = (SynthModule_base *)_object->_cast(Arts::SynthModule_base::_IID);
	if(!module)
		arts_warning("Error using interface %s in the flowsystem: only objects"
					 " implementing Arts::SynthModule should carry streams.",
					 _object->_interfaceName().c_str());
}

StdScheduleNode::StdScheduleNode(Object_skel *object, StdFlowSystem *flowSystem) : ScheduleNode(object)
{
	_object = object;
	this->flowSystem = flowSystem;
	running = false;
	suspended = false;
	module = 0;
	gslModule = 0;
	gslRunning = false;
	queryInitStreamFunc = 0;
	inConn = outConn = 0;
	inConnCount = outConnCount = 0;
}

StdScheduleNode::~StdScheduleNode()
{
	/* stop module if still running */
	if(running) stop();
	/* disconnect all ports */
	stack<Port *> disconnect_stack;

	/*
	 * we must be a bit careful here, as dynamic ports (which are created
	 * for connections by MultiPorts) will suddenly start disappearing, so
	 * we better make a copy of those ports that will stay, and disconnect
	 * them then
	 */
	std::list<Port *>::iterator i;
	for(i=ports.begin();i != ports.end();i++)
	{
		if(!(*i)->dynamicPort()) disconnect_stack.push(*i);
	}

	while(!disconnect_stack.empty())
	{
		disconnect_stack.top()->disconnectAll();
		disconnect_stack.pop();
	}
	/* free them */
	for(i=ports.begin();i != ports.end();i++)
		delete (*i);
	ports.clear();

	freeConn();
}

void StdScheduleNode::initStream(const string& name, void *ptr, long flags)
{
	if(flags == -1)
	{
		queryInitStreamFunc = (QueryInitStreamFunc)ptr;
	}
	else if(flags & streamAsync)
	{
		ports.push_back(new ASyncPort(name,ptr,flags,this));
	}
	else if(flags & streamMulti)
	{
		ports.push_back(new MultiPort(name,ptr,flags,this));
	}
	else
	{
		ports.push_back(new AudioPort(name,ptr,flags,this));
	}

	// TODO: maybe initialize a bit later
	rebuildConn();
}

void StdScheduleNode::addDynamicPort(Port *port)
{
	port->setDynamicPort();
	ports.push_back(port);
	rebuildConn();
}

void StdScheduleNode::removeDynamicPort(Port *port)
{
	std::list<Port *>::iterator i;
	for(i=ports.begin();i!=ports.end();i++)
	{
		Port *p = *i;
		if(p->name() == port->name())
		{
			ports.erase(i);
			rebuildConn();
			return;
		}
	}
}

void StdScheduleNode::start()
{
	assert(!running);
	running = true;

	//cout << "start" << endl;
	accessModule();
	module->streamInit();
	module->streamStart();
	flowSystem->startedChanged();
}

void StdScheduleNode::stop()
{
	assert(running);
	running = false;

	accessModule();
	module->streamEnd();
	flowSystem->startedChanged();
}

void StdScheduleNode::requireFlow()
{
	// cout << "rf" << module->_interfaceName() << endl;
	flowSystem->updateStarted();
	gslMainLoop.run();
}

AutoSuspendState StdScheduleNode::suspendable()
{
	if(running) {
		accessModule();
		return module->autoSuspend();
	}
	// if its not running, who cares?
	return asSuspend;
}

void StdScheduleNode::suspend()
{
	if(running) {
		accessModule();
		suspended = true;
		if((module->autoSuspend() & asSuspendMask) == asSuspendStop) stop();
	}
}

void StdScheduleNode::restart()
{
	if(suspended) {
		accessModule();
		suspended = false;
		if(!running && (module->autoSuspend() & asSuspendMask) == asSuspendStop) start();
	}
}

Port *StdScheduleNode::findPort(const string& name)
{
	std::list<Port *>::iterator i;
	for(i=ports.begin();i!=ports.end();i++)
	{
		Port *p = *i;
		if(p->name() == name) return p;
	}
	if(queryInitStreamFunc)
	{
		if(queryInitStreamFunc(_object,name))
		{
			for(i=ports.begin();i!=ports.end();i++)
			{
				Port *p = *i;
				if(p->name() == name) return p;
			}
		}
	}
	return 0;
}

void StdScheduleNode::virtualize(const std::string& port,
		                         ScheduleNode *implNode,
								 const std::string& implPort)
{
	StdScheduleNode *impl=(StdScheduleNode *)implNode->cast("StdScheduleNode");
	if(impl)
	{
		Port *p1 = findPort(port);
		Port *p2 = impl->findPort(implPort);

		assert(p1);
		assert(p2);
		p1->vport()->virtualize(p2->vport());
	}
}

void StdScheduleNode::devirtualize(const std::string& port,
		                           ScheduleNode *implNode,
								   const std::string& implPort)
{
	StdScheduleNode *impl=(StdScheduleNode *)implNode->cast("StdScheduleNode");
	if(impl)
	{
		Port *p1 = findPort(port);
		Port *p2 = impl->findPort(implPort);

		p1->vport()->devirtualize(p2->vport());
	}
}

void StdScheduleNode::connect(const string& port, ScheduleNode *dest,
		                                          const string& destport)
{
	RemoteScheduleNode *rsn = dest->remoteScheduleNode();
	if(rsn)
	{
		// RemoteScheduleNodes know better how to connect remotely
		rsn->connect(destport,this,port);
		return;
	}

	flowSystem->restart();

	Port *p1 = findPort(port);
	Port *p2 = ((StdScheduleNode *)dest)->findPort(destport);

	if(p1 && p2)
	{
		if((p1->flags() & streamIn) && (p2->flags() & streamOut))
		{
			p1->vport()->connect(p2->vport());
		}
		else if((p2->flags() & streamIn) && (p1->flags() & streamOut))
		{
			p2->vport()->connect(p1->vport());
		}
	}
}

void StdScheduleNode::disconnect(const string& port, ScheduleNode *dest,
		                                             const string& destport)
{
	RemoteScheduleNode *rsn = dest->remoteScheduleNode();
	if(rsn)
	{
		// RemoteScheduleNodes know better how to disconnect remotely
		rsn->disconnect(destport,this,port);
		return;
	}

	flowSystem->restart();

	Port *p1 = findPort(port);
	Port *p2 = ((StdScheduleNode *)dest)->findPort(destport);

	if(p1 && p2)
	{
		if((p1->flags() & streamIn) && (p2->flags() & streamOut))
		{
			p1->vport()->disconnect(p2->vport());
		}
		else if((p2->flags() & streamIn) && (p1->flags() & streamOut))
		{
			p2->vport()->disconnect(p1->vport());
		}
	}
}

AttributeType StdScheduleNode::queryFlags(const std::string& port)
{
	arts_debug("findPort(%s)", port.c_str());
	arts_debug("have %ld ports", ports.size());
	Port *p1 = findPort(port);
	arts_debug("done");

	if(p1)
	{
		arts_debug("result %d",(long)p1->flags());
		return p1->flags();
	}
	arts_debug("failed");
	return (AttributeType)0;
}

void StdScheduleNode::setFloatValue(const string& port, float value)
{
	AudioPort *p = findPort(port)->audioPort();

	if(p) {
		p->vport()->setFloatValue(value);
	} else {
		assert(false);
	}
}

unsigned long StdScheduleNode::inputConnectionCount(const string& port)
{
	unsigned long result = 0;

	unsigned int c;
	for(c = 0; c < inConnCount; c++)
	{
		if(inConn[c]->name() == port)
		{
			if(inConn[c]->source || inConn[c]->gslIsConstant)
				result++;
		}
	}

	return result;
}

unsigned long StdScheduleNode::outputConnectionCount(const string& port)
{
	unsigned long result = 0;

	unsigned int c;
	for(c = 0; c < outConnCount; c++)
	{
		if(outConn[c]->name() == port)
			result += outConn[c]->destcount;
	}

	return result;
}

StdFlowSystem::StdFlowSystem()
{
	_suspended = false;
	needUpdateStarted = false;

	/* TODO: correct parameters */
	static bool gsl_is_initialized = false;
	if(!gsl_is_initialized)
	{
		GslConfigValue values[3] = {
			{ "wave_chunk_padding", 8 },
			{ "dcache_block_size",  4000, },
			{ 0, 0 }
		};
		gsl_is_initialized = true;

		if (!g_thread_supported ())
			g_thread_init(0);
		gsl_init(values, (GslMutexTable *)gslGlobalMutexTable);

		/*
		 * FIXME: both of these are really supposed to be tunable
		 * - the 512 because of low-latency apps, where calculating smaller
		 *   block sizes might be necessary
		 * - the 44100 because of the obvious reason, that not every artsd
		 *   is running at that rate
		 */
		gsl_engine_init(false, 512, 44100, /* subsamplemask */ 63);
		if(gslGlobalMutexTable)
			arts_debug("gsl: using Unix98 pthreads directly for mutexes and conditions");
		/*gsl_engine_debug_enable(GslEngineDebugLevel(GSL_ENGINE_DEBUG_JOBS | GSL_ENGINE_DEBUG_SCHED));*/
	}
	gslMainLoop.initialize();
}

ScheduleNode *StdFlowSystem::addObject(Object_skel *object)
{
	// do not add new modules when being in suspended state
	restart();

	StdScheduleNode *node = new StdScheduleNode(object,this);
	nodes.push_back(node);
	return node;
}

void StdFlowSystem::removeObject(ScheduleNode *node)
{
	StdScheduleNode *xnode = (StdScheduleNode *)node->cast("StdScheduleNode");
	assert(xnode);
	nodes.remove(xnode);
	delete xnode;
}

bool StdFlowSystem::suspended()
{
	return _suspended;
}

bool StdFlowSystem::suspendable()
{
	/*
	 * What it does:
	 * -------------
	 *
	 * The suspension algorithm will first divide the graph of modules into
	 * subgraphs of interconnected modules. A subgraph is suspendable if
	 * all of its modules are suspendable and the subgraph does not contain
	 * producer(s) and consumer(s) at the same time.
	 *
	 * Finally, our module graph is suspendable if all its subgraphs are.
	 *
	 * How it is implemented:
	 * ----------------------
	 *
	 * For efficiency reasons, both steps are merged together. First all
	 * modules will be marked as unseen. Then a module is picked and
	 * all modules that it connects to are recursively added to the
	 * subgraph.
	 */

	/*
	 * initialization: no nodes are seen
	 */
	std::list<StdScheduleNode *>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); i++)
	{
		StdScheduleNode *node = *i;
		node->suspendTag = false;
	}

	stack<StdScheduleNode*> todo;
	for(i = nodes.begin(); i != nodes.end(); i++)
	{
		bool haveConsumer = false;
		bool haveProducer = false;

		/*
		 * examine the subgraph consisting of all nodes connected to (*i)
		 * (only will do anything if suspendTag is not already set)
		 */

		todo.push(*i);
		do
		{
			StdScheduleNode *node = todo.top();
			todo.pop();

			if(!node->suspendTag)
			{
				node->suspendTag = true;	// never examine this node again

				switch (node->suspendable())
				{
					case asNoSuspend|asProducer:
					case asNoSuspend|asConsumer:
					case asNoSuspend:
							return false;
						break;
					case asSuspend:
					case asSuspendStop:	
							/* nothing */
						break;
					case asSuspend|asProducer:
					case asSuspendStop|asProducer:
							if(haveConsumer)
								return false;
							else
								haveProducer = true;
						break;
					case asSuspend|asConsumer:
					case asSuspendStop|asConsumer:
							if(haveProducer)
								return false;
							else
								haveConsumer = true;
						break;
					default:
						arts_fatal("bad suspend value %d", node->suspendable());
				}

				unsigned int c;
				for(c = 0; c < node->inConnCount; c++)
				{
					if(node->inConn[c]->source)
						todo.push(node->inConn[c]->source->parent);
				}
	
				for(c = 0; c < node->outConnCount; c++)
				{
					std::list<Port *>::iterator ci;

					for(ci = node->outConn[c]->autoDisconnect.begin();
						ci != node->outConn[c]->autoDisconnect.end(); ci++)
					{
						AudioPort *dest = (*ci)->audioPort();
						if(dest)
							todo.push(dest->parent);
					}
				}
			}
		} while(!todo.empty());
	}
	return true;
}

void StdFlowSystem::suspend()
{
	if(!_suspended)
	{
		std::list<StdScheduleNode *>::iterator i;
		for(i = nodes.begin();i != nodes.end();i++)
		{
			StdScheduleNode *node = *i;
			node->suspend();
		}
		_suspended = true;
	}
}

void StdFlowSystem::restart()
{
	if(_suspended)
	{
		std::list<StdScheduleNode *>::iterator i;
		for(i = nodes.begin();i != nodes.end();i++)
		{
			StdScheduleNode *node = *i;
			node->restart();
		}
		_suspended = false;
	}
}

/* remote accessibility */

void StdFlowSystem::startObject(Object node)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	sn->start();
}

void StdFlowSystem::stopObject(Object node)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	sn->stop();
}

void StdFlowSystem::connectObject(Object sourceObject,const string& sourcePort,
					Object destObject, const std::string& destPort)
{
	arts_debug("connect port %s to %s", sourcePort.c_str(), destPort.c_str());
	StdScheduleNode *sn =
		(StdScheduleNode *)sourceObject._node()->cast("StdScheduleNode");
	assert(sn);

	Port *port = sn->findPort(sourcePort);
	assert(port);

	StdScheduleNode *destsn =
		(StdScheduleNode *)destObject._node()->cast("StdScheduleNode");
	if(destsn)
	{
		sn->connect(sourcePort,destsn,destPort);
		return;
	}

	ASyncPort *ap = port->asyncPort();

	if(ap)
	{
		FlowSystemSender sender;
		FlowSystemReceiver receiver;
		FlowSystem remoteFs;

		string dest = destObject.toString() + ":" + destPort;
		ASyncNetSend *netsend = new ASyncNetSend(ap, dest);

		sender = FlowSystemSender::_from_base(netsend); // don't release netsend
		remoteFs = destObject._flowSystem();
		receiver = remoteFs.createReceiver(destObject, destPort, sender);
		netsend->setReceiver(receiver);
		arts_debug("connected an asyncnetsend");
	}
}

void StdFlowSystem::disconnectObject(Object sourceObject,
  	const string& sourcePort, Object destObject, const std::string& destPort)
{
	arts_debug("disconnect port %s and %s",sourcePort.c_str(),destPort.c_str());
	StdScheduleNode *sn =
		(StdScheduleNode *)sourceObject._node()->cast("StdScheduleNode");
	assert(sn);

	Port *port = sn->findPort(sourcePort);
	assert(port);

	StdScheduleNode *destsn =
		(StdScheduleNode *)destObject._node()->cast("StdScheduleNode");
	if(destsn)
	{
		sn->disconnect(sourcePort,destsn,destPort);
		return;
	}

	ASyncPort *ap = port->asyncPort();
	if(ap)
	{
		string dest = destObject.toString() + ":" + destPort;
		ap->disconnectRemote(dest);
		arts_debug("disconnected an asyncnetsend");
	}
}

AttributeType StdFlowSystem::queryFlags(Object node, const std::string& port)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	assert(sn);
	return sn->queryFlags(port);
}

void StdFlowSystem::setFloatValue(Object node, const std::string& port,
							float value)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)node._node()->cast("StdScheduleNode");
	assert(sn);
	sn->setFloatValue(port,value);
}

FlowSystemReceiver StdFlowSystem::createReceiver(Object object,
							const string &port, FlowSystemSender sender)
{
	StdScheduleNode *sn =
		(StdScheduleNode *)object._node()->cast("StdScheduleNode");

	Port *p = sn->findPort(port);
	assert(p);

	ASyncPort *ap = p->asyncPort();

	if(ap)
	{
		arts_debug("creating packet receiver");
		return FlowSystemReceiver::_from_base(new ASyncNetReceive(ap, sender));
	}
	return FlowSystemReceiver::null();
}

void StdFlowSystem::updateStarted()
{
	if(!needUpdateStarted)
		return;

	needUpdateStarted = false;

	std::list<StdScheduleNode*>::iterator ni;
	GslTrans *trans = 0;

	for(ni = nodes.begin(); ni != nodes.end(); ni++)
	{
		StdScheduleNode *node = *ni;

		if(node->running != node->gslRunning)
		{
			if(!trans)
				trans = gsl_trans_open();
			gsl_trans_add(trans, gsl_job_set_consumer(node->gslModule, node->running));
			node->gslRunning = node->running;
		}
	}
	if(trans)
		gsl_trans_commit(trans);
}

void StdFlowSystem::startedChanged()
{
	needUpdateStarted = true;
}

// hacked initialization of Dispatcher::the()->flowSystem ;)

namespace Arts {

static class SetFlowSystem : public StartupClass {
	FlowSystem_impl *fs;
public:
	void startup()
	{
		fs = new StdFlowSystem;
		Dispatcher::the()->setFlowSystem(fs);
	}
	void shutdown()
	{
		fs->_release();
	}
} sfs;

}

