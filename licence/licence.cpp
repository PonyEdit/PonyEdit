#include "licence.h"
#include "openssl/rsa.h"
#include "openssl/sha.h"
#include "openssl/bio.h"
#include "openssl/pem.h"
#include <QDebug>
#include <QSettings>
#include "QsLog.h"

char licencePublicKey[] =
	"-----BEGIN PUBLIC KEY-----\n"
	"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7AuOwBjtW4J8xrt+PGKv\n"
	"bOkxDDxk/smi6W2S4g8WIMSNq5GvX4Cg5TgRzzspgkKipZimuF9iQvNEUempmAQy\n"
	"deXARby8PnVtF35mhZomt7X48v57Wgha2nFLpz5/7jabguKyc0n7ox1lRdTfxLWO\n"
	"lzBf4FLRLyDNCXPqCQCmFSV35NPEavVHvdjtX/eTnRF6b2yEdSWT3LEmtMnMuHJT\n"
	"wD5Y1B/UwEv4q3IPO6p4Ebe6VuvwsRuBq9AHS5Jqzi3y7DJwVurrfMx/1eVynSUu\n"
	"VEDHcKkDBhjFK7ciz3Rq0iBHStolrSrwqQT2slb5caxg572HrfBd7A9CBF7j7MKt\n"
	"UwIDAQAB\n"
	"-----END PUBLIC KEY-----\n";

Licence::Licence()
{
	QSettings s;
	setKey(s.value("licence", QVariant()).toByteArray());
}

Licence::Licence(const QByteArray& key)
{
	setKey(key);
}

void Licence::setKey(const QByteArray &key)
{
	mValid = false;
	mVersion = -1;
	mIssueId = -1;
	mKey = key;

	//	Step 1: base64-decode and separate license from signature
	QByteArray decodedKey = QByteArray::fromBase64(key);
	int signatureDivider = decodedKey.indexOf('/');
	if (signatureDivider == -1) return;
	QByteArray signature = decodedKey.mid(signatureDivider + 1);
	decodedKey.truncate(signatureDivider);

	//	Step 2: verify the signature matches the licence key
	if (!verifySignature(decodedKey, signature))
		return;

	//	Step 3: read the licence key details
	QList<QByteArray> pieces = decodedKey.split(':');
	if (pieces.length() < 4) return;

	//	Piece 1: User login
	mLogin = pieces[0];

	//	Piece 2: Maximum major version number licenced
	bool ok;
	mVersion = pieces[1].toInt(&ok);
	if (!ok) return;

	//	Piece 3: Expiry date
	if (pieces[2].length())
		mExpiry = QDate::fromString(pieces[2], Qt::ISODate);
	else
		mExpiry = QDate();

	//	Piece 4: Arbitrary issue id
	mIssueId = pieces[3].toInt(&ok);
	if (!ok) return;

	//	If it reaches here, having passed the signature test and read all parts, this license is valid.
	mValid = true;
}

bool Licence::verifySignature(const QByteArray& data, const QByteArray& signature)
{
	//	Calculate the SHA1 hash of data
	SHA_CTX sha_ctx = { 0, 0, 0, 0, 0, 0, 0, { 0 }, 0 };
	unsigned char digest[SHA_DIGEST_LENGTH];
	if (SHA1_Init(&sha_ctx) != 1) { QLOG_ERROR() << "Failed to init SHA1 libs"; return false; }
	if (SHA1_Update(&sha_ctx, data.constData(), data.length()) != 1) { QLOG_ERROR() << "Failed to add data to SHA1 hash"; return false; }
	if (SHA1_Final(digest, &sha_ctx) != 1) { QLOG_ERROR() << "Failed to finalize SHA1 hash"; return false; }

	//	Prepare the Licence Public Key
	BIO* b = BIO_new_mem_buf(licencePublicKey, strlen(licencePublicKey));
	EVP_PKEY* k = PEM_read_bio_PUBKEY(b, NULL, NULL, NULL);

	//	Verify the signature
	bool result = RSA_verify(NID_sha1, digest, sizeof(digest), (unsigned char*)signature.constData(), signature.length(), EVP_PKEY_get1_RSA(k));

	//	Clean up
	EVP_PKEY_free(k);
	BIO_free(b);

	return result;
}

void Licence::save()
{
	QSettings settings;
	settings.setValue("licence", mKey);
}


