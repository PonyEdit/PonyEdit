#ifndef SYNTAXDEFINITION_H
#define SYNTAXDEFINITION_H

#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QtXml>
#include "main/stringtrie.h"

class SyntaxRule;
class SyntaxDefinition {
	public:
		struct CommentStyle {
			bool multiline;
			QString start;
			QString end;
		};

		struct ContextDef;
		struct ContextLink {
			ContextLink() :
				popCount( 0 ),
				contextDef( nullptr ) {}
			int popCount;
			QSharedPointer< ContextDef > contextDef;
		};

		struct ItemData {
			QString name;
			QString styleName;
			QString styleNameLower;
			QString color;
			QString selColor;
			bool italic;
			bool bold;
			bool underline;
			bool strikeout;
		};

		struct ContextDef {
			ContextDef();
			~ContextDef();

			QString attribute;
			QString name;
			QString lineEndContext;
			QString lineBeginContext;
			bool fallthrough;
			QString fallthroughContext;
			bool dynamic;
			QList< QSharedPointer< SyntaxRule > > rules;

			ContextLink fallthroughContextLink;
			ContextLink lineBeginContextLink;
			ContextLink lineEndContextLink;

			int listIndex;
			ItemData *attributeLink;
		};

		struct KeywordList {
			QString name;
			StringTrie items;
			StringTrie lcItems;     // For case-insensitive matching.
		};

		SyntaxDefinition( const QString &filename );
		~SyntaxDefinition() {
			unlink();
		}

		inline bool isValid() const {
			return mValid;
		}

		inline QSharedPointer< ContextDef > getContextByIndex( int index ) const {
			return mContextList.at( index );
		}

		inline QSharedPointer< ContextDef > getDefaultContext() const {
			return mDefaultContext;
		}

		inline QSharedPointer< ContextDef > getContext( const QString &name ) const {
			return mContextMap.value( name.toLower() );
		}

		inline KeywordList *getKeywordList( const QString &name ) const {
			return mKeywordLists.value( name.toLower() );
		}

		inline ItemData *getItemData( const QString &name ) const {
			return mItemDatas.value( name.toLower() );
		}

		inline const QString &getSyntaxName() const {
			return mSyntaxName;
		}

		void addKeywordList( KeywordList *list );
		void addContext( ContextDef *context );
		void addItemData( ItemData *itemData );

		inline void setIndentationSensitive( bool v ) {
			mIndentationSensitive = v;
		}

		inline void setCaseSensitiveKeywords( bool v ) {
			mCaseSensitiveKeywords = v;
		}

		void setWeakDeliminators( const QString &v );
		inline void setAdditionalDeliminators( const QString &v ) {
			mAdditionalDeliminators = v; mDeliminators.append( v );
		}

		inline void setWordWrapDeliminator( const QString &v ) {
			mWordWrapDeliminator = v;
		}

		inline void setSyntaxName( const QString &n ) {
			mSyntaxName = n;
		}

		inline bool isDeliminator( const QChar &c ) {
			return mDeliminators.contains( c );
		}

		bool linkContext( const QString &context, ContextLink *link );

		Qt::CaseSensitivity getKeywordCaseSensitivity() {
			return mCaseSensitiveKeywords ? Qt::CaseSensitive : Qt::CaseInsensitive;
		}

	private:
		bool link();
		void unlink();

		bool mValid;
		QString mSyntaxName;

		QMap< QString, KeywordList * > mKeywordLists;
		QMap< QString, QSharedPointer< ContextDef > > mContextMap;
		QSharedPointer< ContextDef > mDefaultContext;
		QMap< QString, ItemData * > mItemDatas;

		QList< QSharedPointer< ContextDef > > mContextList;

		bool mIndentationSensitive;
		bool mCaseSensitiveKeywords;
		QString mWeakDeliminators;
		QString mAdditionalDeliminators;
		QString mWordWrapDeliminator;
		QString mDeliminators;
		QList< CommentStyle > mCommentStyles;
};

typedef QSharedPointer< SyntaxDefinition::ContextDef > ContextDefLink;

#endif  // SYNTAXDEFINITION_H
