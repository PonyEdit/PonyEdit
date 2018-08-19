#include <QFile>
#include <QtXml>
#include "main/tools.h"
#include "QsLog.h"
#include "syntaxdefinition.h"
#include "syntaxdefxmlhandler.h"
#include "syntaxrule.h"

SyntaxDefinition::ContextDef::ContextDef() :
	fallthrough( false ),
	dynamic( false ),
	listIndex( 0 ),
	attributeLink( nullptr ) {}
SyntaxDefinition::ContextDef::~ContextDef() = default;

SyntaxDefinition::SyntaxDefinition( const QString &filename ) :
	mValid( false ),
	mSyntaxName(),
	mIndentationSensitive( false ),
	mCaseSensitiveKeywords( false ),
	mWeakDeliminators(),
	mAdditionalDeliminators(),
	mWordWrapDeliminator(),
	mDeliminators( ".():!+,-<=>%&/;?[]^{|}~\\*, \t" ) {
	QFile file( filename );
	if ( file.open( QFile::ReadOnly ) ) {
		SyntaxDefXmlHandler handler( this );
		QXmlSimpleReader reader;
		QXmlInputSource source( &file );
		reader.setContentHandler( &handler );
		reader.setErrorHandler( &handler );

		if ( reader.parse( &source ) ) {
			if ( link() ) {
				mValid = true;
			}
		}
	}

	if ( ! mValid ) {
		QLOG_ERROR() << "Failed to read syntax definition";
	}
}

void SyntaxDefinition::unlink() {
	foreach ( KeywordList *list, mKeywordLists ) {
		delete list;
	}

	foreach ( ItemData *itemData, mItemDatas ) {
		delete itemData;
	}

	foreach ( QSharedPointer< ContextDef > context, mContextList ) {
		context->fallthroughContextLink = ContextLink();
		context->lineBeginContextLink = ContextLink();
		context->lineEndContextLink = ContextLink();

		foreach ( QSharedPointer< SyntaxRule > rule, context->rules ) {
			rule->unlink();
		}
	}
}

bool SyntaxDefinition::link() {
	// Go through the rules in all contexts
	foreach ( QSharedPointer< ContextDef > context, mContextList ) {
		// Link up this context's fallthough, lineEnd, lineBegin references (if there is one)
		if ( context->fallthrough ) {
			linkContext( context->fallthroughContext, &context->fallthroughContextLink );
		}
		if ( ! context->lineBeginContext.isEmpty() ) {
			linkContext( context->lineBeginContext, &context->lineBeginContextLink );
		}
		if ( ! context->lineEndContext.isEmpty() ) {
			linkContext( context->lineEndContext, &context->lineEndContextLink );
		}

		// Link up this context's attribute property (if there is one)
		if ( ! context->attribute.isEmpty() ) {
			context->attributeLink = getItemData( context->attribute );
		}

		// Pick through the rules in this context, linking <IncludeRules>, context="", etc.
		for ( int i = 0; i < context->rules.length(); i++ ) {
			QSharedPointer< SyntaxRule > rule = context->rules[ i ];

			// Deal with <IncludeRules> tags
			if ( rule->getType() == SyntaxRule::IncludeRules ) {
				// Remove the tag...
				context->rules.removeAt( i );

				// Parse it.
				QString source = rule->getContext();
				QSharedPointer< ContextDef > sourceContext;
				int pos;
				if ( source.startsWith( "##" ) ) {
					SyntaxDefinition *includedDefinition =
						gSyntaxDefManager->getDefinitionForSyntax( source.mid( 2 ) );
					if ( includedDefinition ) {
						sourceContext = includedDefinition->getDefaultContext();
					}
				} else if ( ( pos = source.indexOf( "##" ) ) != -1 ) {
					SyntaxDefinition *includedDefinition =
						gSyntaxDefManager->getDefinitionForSyntax( source.mid( pos + 2 ) );
					if ( includedDefinition ) {
						sourceContext = includedDefinition->getContext( source.left( pos ) );
					}
				} else {
					sourceContext = getContext( source );
				}

				if ( sourceContext.isNull() ) {
					QLOG_WARN() << "Warning: IncludeRule names non-existent context: " << source;
				} else {
					if ( rule->getIncludeAttrib() ) {
						context->attribute = sourceContext->attribute;
						context->attributeLink = sourceContext->attributeLink;
					}

					int insertionOffset = 0;
					foreach ( QSharedPointer< SyntaxRule > copyRule, sourceContext->rules ) {
						context->rules.insert( i + insertionOffset++, copyRule );
					}
				}

				i--;
			} else if ( ! rule->link( this ) ) {
				return false;
			}
		}
	}

	return true;
}

bool SyntaxDefinition::linkContext( const QString &context, ContextLink *link ) {
	if ( context.startsWith( '#' ) || context.isEmpty() ) {
		if ( context.startsWith( "##" ) ) {
			SyntaxDefinition *includedDefinition = gSyntaxDefManager->getDefinitionForSyntax( context.mid(
														  2 ) );
			if ( includedDefinition ) {
				link->contextDef = includedDefinition->getDefaultContext();
			} else {
				QLOG_WARN() << "Link to non-existant external syntax def: " << context;
				return false;
			}
		}
		if ( context.startsWith( "#pop", Qt::CaseInsensitive ) ) {
			link->popCount = context.count( '#' );
		}
	} else {
		link->contextDef = getContext( context );
		if ( ! link->contextDef ) {
			QLOG_WARN() << "Failed to link syntax context: " << context;
			return false;
		}
	}

	return true;
}

void SyntaxDefinition::addKeywordList( KeywordList *list ) {
	mKeywordLists.insert( list->name.toLower(), list );
}

void SyntaxDefinition::addContext( ContextDef *context ) {
	QSharedPointer< ContextDef > wrappedContext = QSharedPointer< ContextDef >( context );

	if ( mDefaultContext.isNull() ) {
		mDefaultContext = wrappedContext;
	}
	mContextMap.insert( context->name.toLower(), wrappedContext );
	mContextList.append( wrappedContext );
}

void SyntaxDefinition::addItemData( ItemData *itemData ) {
	mItemDatas.insert( itemData->name.toLower(), itemData );
}

void SyntaxDefinition::setWeakDeliminators( const QString &v ) {
	mWeakDeliminators = v;
	for ( auto i : v ) {
		mDeliminators.remove( i );
	}
}
