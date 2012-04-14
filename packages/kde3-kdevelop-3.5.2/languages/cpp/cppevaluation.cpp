/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cppevaluation.h"
#include "simplecontext.h"
#include "safetycounter.h"

extern SafetyCounter safetyCounter;

namespace CppEvaluation {

void statusLog( const QString& str ) {
  ifVerboseMajor( dbgMajor() << str << endl );
  statusBarText( str, 2000 );
}
OperatorSet AllOperators;

///These lines register the operators to the list of all operators
RegisterOperator< DotOperator > DotReg( AllOperators );
RegisterOperator< NestedTypeOperator > NestedReg( AllOperators );
RegisterOperator< ArrowOperator > ArrowReg( AllOperators );
RegisterOperator< StarOperator > StarReg( AllOperators );
RegisterOperator< AddressOperator > AddressReg( AllOperators );
RegisterOperator< IndexOperator > IndexReg( AllOperators );
RegisterOperator< ParenOperator > ParenReg( AllOperators );

template<class To, class From>  
QValueList<To> convertList( const QValueList<From>& from ) {
  QValueList<To> ret;
  for( typename QValueList<From>::const_iterator it = from.begin(); it != from.end(); ++it ) {
    ret << (To)*it;
  }
  return ret;
}

QString nameFromType( SimpleType t ) {
  return t->fullTypeResolved();
}

QString Operator::printTypeList( QValueList<EvaluationResult>& lst )
{
  QString ret;
  for( QValueList<EvaluationResult>::iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret += "\"" + (*it)->fullNameChain() + "\", ";
  }
  ret.truncate( ret.length() - 3 );
  return ret;
}

void Operator::log( const QString& msg ) {
  statusLog( "\"" + name() + "\": " + msg );
  //ifVerboseMajor( dbgMajor() << "\"" << name() << "\": " << msg << endl );
}

OperatorSet::~OperatorSet() {
  for( QValueList< Operator* >::iterator it = m_operators.begin(); it != m_operators.end(); ++it ) {
    delete *it;
  }
}

OperatorIdentification OperatorSet::identifyOperator( const QString& str_ , Operator::BindingSide allowedBindings) {
  QString str = str_.stripWhiteSpace();
  for( OperatorList::iterator it = m_operators.begin(); it != m_operators.end(); ++it ) {
    if( ((*it)->binding() & allowedBindings) == (*it)->binding() ) {
      if( OperatorIdentification ident = (*it)->identify( str ) ) {
        return ident;
      }
    }
  }
  
  return OperatorIdentification();
}

OperatorIdentification UnaryOperator::identify( QString& str ) {
  OperatorIdentification ret;
  if( str.startsWith( m_identString ) ) {
    ret.start = 0;
    ret.end = m_identString.length();
    ret.found = true;
    ret.op = this;
  }
  return ret;
}

EvaluationResult UnaryOperator::apply( QValueList<EvaluationResult> params, QValueList<EvaluationResult> innerParams ) {
  if( !checkParams( params ) ) {
  log( QString("parameter-check failed: %1 params: ").arg( params.size() ) + printTypeList( params ) );
    return EvaluationResult();
  } else {
    EvaluationResult t = unaryApply( params.front(), innerParams );
    if( !t ) {
      if( params.front() )
        log( "could not apply \"" + name() + "\" to \"" + nameFromType( params.front() ) + "\"");
      else
        log( "operator \"" + name() + "\" applied on \"" + nameFromType( params.front() )  + "\": returning unresolved type \"" + nameFromType( t ) + "\"");
    }
    return t;
  }
}

EvaluationResult NestedTypeOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
  return param;
}

EvaluationResult DotOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
  if( param->totalPointerDepth() == 0 ) {
    return param;
  } else {
    log( "failed to apply dot-operator to " + param->fullNameChain() + " because the pointer-depth is wrong" );
    return EvaluationResult();
  }
}


EvaluationResult ArrowOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
  if( param->totalPointerDepth() == 1 ) {
    param->setTotalPointerDepth( param->totalPointerDepth() - 1 );
    return param;
  } else {
    if( param->resolved() ) {
      if( param->totalPointerDepth() == 0 ) {
          return param->resolved()->applyOperator( SimpleTypeImpl::ArrowOp , convertList<LocateResult, EvaluationResult>(innerParams) );
      } else {
          log("failed to apply arrow-operator to " + param->fullNameChain() + " because the pointer-depth is wrong" );
          return EvaluationResult();
      }
    } else {
      log( "failed to apply arrow-operator to unresolved type" );
      return EvaluationResult();
    }
  };
}

EvaluationResult StarOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
  if( param->totalPointerDepth() > 0 ) {
    param->setTotalPointerDepth( param->totalPointerDepth() - 1 );
    return param;
  } else {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::StarOp );
    } else {
      log( "failed to apply star-operator to unresolved type" );
      return EvaluationResult();
    }
  };
}

EvaluationResult AddressOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
  param->setTotalPointerDepth( param->totalPointerDepth() + 1 );
  return param;
}

OperatorIdentification UnaryParenOperator::identify( QString& str ) {
  OperatorIdentification ret;
  if( str.startsWith( QString( identString()[0] ) ) ) {
    ret.start = 0;
    ret.end = findClose( str, 0 );
    if( ret.end == -1 ) {
      ret.found = false;
      ret.end = 0;
    } else {
      if( str[ret.end] == identString()[1] ) {
        ret.found = true;
        ret.end += 1;
        ret.op = this;
        
                    ///Try to extract the parameter-strings.
        ParamIterator it( identString(), str.mid( ret.start, ret.end - ret.start ) );
        
        while( it ) {
          ret.innerParams << (*it).stripWhiteSpace();
          
          ++it;
        }
        
      } else {
        ret.end = 0;
      }
    }
  }
  return ret;
}


EvaluationResult IndexOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
  if( param->totalPointerDepth() > 0 ) {
    param->setTotalPointerDepth( param->totalPointerDepth() - 1 );
    return param;
  } else {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::IndexOp, convertList<LocateResult>( innerParams ) );
    } else {
      log( "failed to apply index-operator to unresolved type" );
      return EvaluationResult();
    }
  };
}

EvaluationResult ParenOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
  if( param ) {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::ParenOp, convertList<LocateResult>(innerParams) );
    } else {
      log( "failed to apply paren-operator to unresolved type" );
      return EvaluationResult();
    }
    
  } else {
    return innerParams[0];
  }
}

ExpressionEvaluation::ExpressionEvaluation( CppCodeCompletion* data, ExpressionInfo expr, OperatorSet& operators, const HashedStringSet& includeFiles, SimpleContext* ctx ) : m_data( data ), m_ctx( ctx ), m_expr( expr ), m_global(false), m_operators( operators ), m_includeFiles( includeFiles ) {
  safetyCounter.init();
  
  ifVerboseMajor( dbgMajor( ) << "Initializing evaluation of expression " << expr << endl );
  
  if ( expr.expr().startsWith( "::" ) )
  {
    expr.setExpr( expr.expr().mid( 2 ) );
    m_global = true;
  }
  
          //m_expr = m_data->splitExpression( expr.expr() ).join("");
}

EvaluationResult ExpressionEvaluation::evaluate() {
  EvaluationResult res;
  res = evaluateExpressionInternal( m_expr.expr(), m_ctx->global(), m_ctx, m_ctx, /*m_expr.canBeTypeExpression() cannot be safely determined*/true );
  
  ExpressionInfo ex = res.expr; ///backup and set the type which was chosen while the evaluation-process
  res.expr = m_expr;
  res.expr.t = ex.t;
  
  return res;
}

EvaluationResult ExpressionEvaluation::evaluateExpressionInternal( QString expr, EvaluationResult scope, SimpleContext * ctx, SimpleContext* innerCtx , bool canBeTypeExpression) {
  LogDebug d( "#evl#" );
  if( expr.isEmpty() || !safetyCounter ) {
    scope.expr.t = ExpressionInfo::NormalExpression;
    return scope;
  }
  
	/*if( !scope->resolved() ) {
    ifVerboseMajor( dbgMajor() << "evaluateExpressionInternal(\"" << expr << "\") scope: \"" << scope->fullTypeStructure() << "\" is unresolved " << endl );
    return EvaluationResult();
  }*/
  
  ifVerboseMajor( dbgMajor() << "evaluateExpressionInternal(\"" << expr << "\") scope: \"" << scope->fullNameChain() << "\" context: " << ctx << endl );
  
  expr = expr.stripWhiteSpace();
  
        ///Find the rightmost operator with the lowest priority, for the first split.
  QValueList<OperatorIdentification> idents;
  for( uint a = 0; a < expr.length(); ++a ) {
    QString part = expr.mid( a );
    OperatorIdentification ident = m_operators.identifyOperator( part );
    if( ident ) {
      ifVerboseMajor( dbgMajor() << "identified \"" << ident.op->name() << "\" in string " << part << endl );
      ident.start += a;
      ident.end += a;
      idents << ident;
      a += ident.end;
    } else {
      if( isLeftParen( part[0] ) ) {
        int jump = findClose( part, 0 );
        if( jump != -1 )
          a += jump;
      }
    }
  }
  
  if( !idents.isEmpty() ) {
    OperatorIdentification lowest;
    
    for( QValueList<OperatorIdentification>::iterator it = idents.begin(); it != idents.end(); ++it ) {
      if( lowest ) {
        if( lowest.op->priority() >= (*it).op->priority() )
          lowest = *it;
      } else {
        lowest = *it;
      }
    }
    
    if( lowest ) {
      QString leftSide = expr.left( lowest.start ).stripWhiteSpace();
      QString rightSide = expr.right( expr.length() - lowest.end ).stripWhiteSpace();
      
      EvaluationResult left, right;
      if( !leftSide.isEmpty() ) {
	      left = evaluateExpressionInternal( leftSide, scope, ctx, innerCtx, lowest.op->canBeType( Operator::Left ) );
      } else {
        left = scope;
      }
      
      if( !left && (lowest.op->binding() & Operator::Left) ) {
        ifVerboseMajor( dbgMajor() << "problem while evaluating expression \"" << expr << "\", the operator \"" << lowest.op->name() << "\" has a binding to the left side, but no left side could be evaluated: \"" << leftSide << "\"" << endl );
      }
      
      if( !rightSide.isEmpty() && (lowest.op->binding() & Operator::Right) )
	      right = evaluateExpressionInternal( rightSide, SimpleType(), ctx, innerCtx, lowest.op->canBeType( Operator::Right ) );
      
      if( !right && (lowest.op->binding() & Operator::Right) ) {
        ifVerboseMajor( dbgMajor() << "problem while evaluating expression \"" << expr << "\", the operator \"" << lowest.op->name() << "\" has a binding to the right side, but no right side could be evaluated: \"" << rightSide << "\"" << endl );
      }
      
      QValueList<EvaluationResult> innerParams;
      QValueList<EvaluationResult> params;
      if( lowest.op->binding() & Operator::Left ) params << left;
      if( lowest.op->binding() & Operator::Right ) params << right;
      
      for( QValueList<QString>::iterator it = lowest.innerParams.begin(); it != lowest.innerParams.end(); ++it ) {
        ifVerboseMajor(dbgMajor() << "evaluating inner parameter \"" + *it + "\"" );
	      innerParams << evaluateExpressionInternal( (*it), SimpleType(), innerCtx, innerCtx, lowest.op->canBeType( Operator::Neutral ) );
      }
      
      EvaluationResult applied = lowest.op->apply( params, innerParams );
      if( !applied ) {
        statusLog( "\"" + expr + "\": failed to apply the operator \"" + lowest.op->name() + "\"" );
      }
      
      if( ! (lowest.op->binding() & Operator::Left) &&  !leftSide.isEmpty() ) {
                    ///When the operator has no binding to the left, the left side should be empty.
        statusLog( "\"" + expr + "\": problem with the operator \"" + lowest.op->name() + ", it has no binding to the left side, but the left side is \""+ leftSide + "\"" );
      }
      
      if( ! (lowest.op->binding() & Operator::Right) && !rightSide.isEmpty() ) {
				///When the operator has no binding to the right, we should continue evaluating the right side, using the left type as scope.
	      ///Think about this.
	      return evaluateExpressionInternal( rightSide, applied, 0, innerCtx, lowest.op->canBeType( Operator::Right ) );
      }
      
      return applied;
    } else {
      ifVerboseMajor( dbgMajor() << " could not find an operator in " << expr << endl );
      return evaluateAtomicExpression( expr, scope, ctx );
    }
  }
  
        //dbgMajor() << " could not evaluate " << expr << endl;
  ifVerboseMajor( dbgMajor() << "evaluating \"" << expr << "\" as atomic expression" << endl );

  TypeDesc exp = m_ctx->container()->resolveTemplateParams( TypeDesc(expr) );

	ifVerboseMajor( dbgMajor() << "after template-parameter resolution: \"" << exp.fullNameChain() << "\"" << endl );
	
	EvaluationResult res = evaluateAtomicExpression( exp, scope, ctx, canBeTypeExpression );
  return res;
}

/**This function needs a clean workover.
 * An atomic expression is one that only consists of a type-, function- or variable-name(may include '::')
 */
EvaluationResult ExpressionEvaluation::evaluateAtomicExpression( TypeDesc expr, EvaluationResult scope, SimpleContext * ctx, bool canBeTypeExpression ) {
  LogDebug d( "#evt#");
  if( !safetyCounter || !d ) return SimpleType();
	bool canBeItemExpression = true; ///To be implemented

    if( scope ) {
        expr.setIncludeFiles( scope.resultType->includeFiles() );
    } else {
        expr.setIncludeFiles( m_includeFiles );
    }
 
  
  ifVerboseMajor( dbgMajor() << "evaluateAtomicExpression(\"" << expr.name() << "\") scope: \"" << scope->fullNameChain() << "\" context: " << ctx << endl );

	EvaluationResult bestRet;
  int bestDepth = 0;
	
  if( expr.name().isEmpty() )
    return scope;
  
  TypePointer searchIn = scope->resolved();
  if( !searchIn ) {
    statusLog( "scope-type is not resolved" );
    return EvaluationResult();
  }

	if( ctx )
		searchIn = ctx->container().get();
	
	if( ctx && canBeItemExpression ) {
		///Search for variables and functions, first in the current context, and then through the container-classes upwards.
			// find the variable in the current context
		SimpleVariable var = ctx->findVariable( expr.name() );
	
    if ( var.type ) {
        TypeDesc d( var.type );
        d.setIncludeFiles( m_includeFiles );
			EvaluationResult ret = EvaluationResult( ctx->container()->locateDecType( d ), var.toDeclarationInfo( "current_file" ));
      ret.expr.t = ExpressionInfo::NormalExpression;
      return ret;
    }
	
		SimpleType current = ctx->container();
	
		SimpleTypeImpl::TypeOfResult type;
	
		SafetyCounter s( 20 );
		bool ready = false;
    int depth = 0; 
    
		while( !ready && s )
		{
			if( !current ) ready = true;
	
			type = current->typeOf( expr );
      if ( type) {
				bestRet = EvaluationResult( type.type, type.decl );
        bestDepth = depth;
        bestRet.expr = expr.fullNameChain();
        bestRet.expr.t = ExpressionInfo::NormalExpression;
      }

      depth++;
			if( !ready ) current = current->parent();
		}
	}
	/*
  if( scope.expr.t & ExpressionInfo::TypeExpression )
    canBeTypeExpression = true;*/

	if( canBeItemExpression && (!bestRet || bestDepth > 0 ) ) {
		//SimpleTypeImpl::
		SimpleTypeImpl::TypeOfResult res = searchIn->typeOf( expr );
    
    if( res ) {
      bestRet = EvaluationResult( res.type, res.decl );
      bestDepth = 0;
    }
  }
	if( canBeTypeExpression ) {
		///Search for Types
		LocateResult type = searchIn->locateDecType( expr );

		if( !bestRet ||
		/** Did we find a constructor within a class? */
		(type->resolved() && ( bestRet->resolved() && type->resolved()->desc() == bestRet->resolved()->parent()->desc() && bestRet->resolved()->asFunction() ) ) ) {
    /*if ( type && type->resolved() )
    {*/
      EvaluationResult ret = type;
      ret.expr = expr.fullNameChain();
      ret.expr.t = ExpressionInfo::TypeExpression;
      bestRet = ret;
    }
    /*} else {
			bestRet = EvaluationResult( type );
			QStringList s = split+exprList;
			s.pop_front();
			if( !s.isEmpty() )
				bestRet->append( new TypeDescShared( s.join("::") ) );
    }*/
	}

  if( bestRet )
    return bestRet;
	
  ifVerboseMajor( dbgMajor() << "evaluateAtomicExpression: \"" << scope.resultType->fullNameChain() << "\"could not locate " << expr.fullNameChain() << endl );
  return  bestRet;
}
}

// kate: indent-mode csands; tab-width 2;
