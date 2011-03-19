import time
import binascii
import urlparse, urllib, urllib2
import socket
from sys import exc_info
from zlib import compress, decompress

from zLOG import LOG, INFO
from cPickle import Pickler, Unpickler
from cStringIO import StringIO
from AccessControl import ClassSecurityInfo
from DateTime import DateTime
from BTrees.OOBTree import OOBTree, OOSet
from ZODB import Persistent
from ZODB.PersistentMapping import PersistentMapping
from zLOG import INFO
from Acquisition import ImplicitAcquisitionWrapper
from Acquisition import aq_base

from bases import CremeSimpleItem, CremeContainer
from Products.Associations.IReferenceWrappers import IReferenceListWrapper
from utils import zlog

def oid2str(oid):
    return binascii.b2a_hex(oid).upper()

class ReplogTransaction(Persistent):

    meta_type = 'ReplogTransaction'

    def __init__(self, transaction_id):
        self.id = transaction_id
        self._entries = OOBTree()
        t = get_transaction()
        self.user = t.user
        self.description = t.description

    def add_entry(self, entry_dict):
        self._entries[entry_dict.get('id')] = entry_dict

    def replicate(self, target):
        d = {'id': self.id,
             'user': self.user, 
             'description': self.description,
             'entries': self._entries}
        f = StringIO()
        p = Pickler(f)
        p.dump(d)

        payloadStr = f.getvalue()
        LOG('Replication', INFO, 'replicate> transaction id: %s; '
                'size (uncompressed): %s' % (
                    oid2str(self.id), len(payloadStr))) #DBG
        payloadStr = compress(payloadStr)

        handler = FixedHTTPHandler()
        opener = urllib2.build_opener(handler)
        urllib2.install_opener(opener)

        LOG('Replication', INFO, 'replicate> transaction id: %s; size: %s' % (
            oid2str(self.id), len(payloadStr))) #DBG
        url = '%s/load' % target.url
        schema, domain, path, x1, x2, x3 = urlparse.urlparse(url)
        newurl = '%s://%s:%s@%s%s' % (
            schema, target.username, target.password, domain, path)
        try:
            urllib2.urlopen(newurl, urllib.urlencode({'data': payloadStr}))
        except urllib2.HTTPError, e:
            if e.code != 204: # 204 == 'No content' which is what we expect
                raise

class FixedHTTPHandler(urllib2.HTTPHandler):
    """ urllib2.HTTPHandler does not handle http://usn:pass@host/path
        urls
    """

    def do_open(self, http_class, req):
        host = req.get_host()
        if not host:
            raise URLError('no host given')

        user_passwd, host = urllib.splituser(host)
        host = urllib.unquote(host)
        if user_passwd:
            import base64
            auth = base64.encodestring(user_passwd).strip()
        else:
            auth = None

        h = http_class(host) # will parse host:port
        if req.has_data():
            data = req.get_data()
            h.putrequest('POST', req.get_selector())
            if not req.headers.has_key('Content-type'):
                h.putheader('Content-type',
                            'application/x-www-form-urlencoded')
            if not req.headers.has_key('Content-length'):
                h.putheader('Content-length', '%d' % len(data))
        else:
            h.putrequest('GET', req.get_selector())

        if auth: h.putheader('Authorization', 'Basic %s' % auth)

        scheme, sel = urllib.splittype(req.get_selector())
        sel_host, sel_path = urllib.splithost(sel)
        h.putheader('Host', sel_host or host)
        for args in self.parent.addheaders:
            h.putheader(*args)
        for k, v in req.headers.items():
            h.putheader(k, v)
        # httplib will attempt to connect() here.  be prepared
        # to convert a socket error to a URLError.
        try:
            h.endheaders()
        except socket.error, err:
            raise urllib2.URLError(err)
        if req.has_data():
            h.send(data)

        code, msg, hdrs = h.getreply()
        fp = h.getfile()
        if code == 200:
            return urllib.addinfourl(fp, hdrs, req.get_full_url())
        else:
            return self.parent.error('http', req, fp, code, msg, hdrs)


class ReplicationTarget(CremeSimpleItem):

    meta_type = 'ReplicationTarget'

    _properties = (
        {'id':'url', 'type':'string', 'mode':'wd'},
        {'id':'username', 'type':'string', 'mode':'wd'},
        {'id':'password', 'type':'string', 'mode':'wd'},
    )

    def __init__(self, **kw):
        self._transactions = OOBTree()
        CremeSimpleItem.__init__(self, **kw)

    def log(self, entry_dict):
        t_id = self._p_jar._storage._serial
        if not self._transactions.has_key(t_id):
            self._transactions[t_id] = ReplogTransaction(t_id)
        transaction = self._transactions[t_id]
        transaction.add_entry(entry_dict)


    def replicate(self):
        """ replicate log to targets """
        keys = OOSet(self._transactions.keys())
        for key in keys:
            transaction = self._transactions[key]
            try:
                transaction.replicate(self)
                del self._transactions[key]
            except:
                from sys import exc_info
                import traceback
                info = exc_info()
                zlog('Replication',
                     'Could not replicate transaction %s to %s'%(
                        oid2str(transaction.id), self.id)) 
                break

    def html(self, suppress_entries=0):
        """ html log for viewing transactions in the ZMI """
        out = []
        keys = OOSet(self._transactions.keys())
        for t_id in keys:
            t = self._transactions[t_id]
            out.append('''
<h4>Transaction id: %s</h4>
<p>
<em>User:</em> %s<br/>
<em>Description:</em> %s<br/>
</p>
''' % (oid2str(t.id), t.user, t.description))
            if suppress_entries:
                continue
            for entry_id in t._entries.keys():
                entry = t._entries[entry_id]
                out.append('''
<p>
<em>id:</em> %(id)s<br/>
<em>obj:</em> %(path)s<br/>
<em>method:</em> %(method)s<br/>
<em>args:</em> %(args)s<br/>
</p>
''' % entry)
        out = '<hr>'.join(out)
        return '<html><body>%s</body></html>' % out

class ReplicationLog(CremeContainer):
    """ Replicates log entries to replication targets """

    meta_type = 'ReplicationLog'

    security = ClassSecurityInfo()
    security.declareObjectProtected('Use ReplicationLog')

    def __init__(self, **kw):
        self.id = kw.get('id', 'ReplicationLog')
        self._last_transaction_id = 0
        self._last_entry_id = 0

    def _get_entry_id(self):
        # XXX This check can go after the method has been called once.
        if not getattr(self, '_last_entry_id'):
            self._last_entry_id = 0
        new_id = str(time.time())
        while new_id == self._last_entry_id:
            new_id = str(time.time())
        self._last_entry_id = new_id
        return new_id

    def log(self, obj, method, meth_args):
        if obj.meta_type in ['ReplicationTarget', 'ReplicationLog']:
            return

        entry_dict = {}
        entry_dict['id'] = self._get_entry_id()
        entry_dict['path'] = self._object_path(obj)
        entry_dict['method'] = method
        entry_dict['args'] = self._serialize_args(meth_args) 
        zLOG.LOG('Replication', INFO, 'ReplicationLog.log> entry_dict: %s' % entry_dict) #DBG 

        for target in self.objectValues('ReplicationTarget'):
            target.log(entry_dict)

    def replicate(self):
        """ replicate log to targets """
        for target in self.objectValues('ReplicationTarget'):
            target.replicate()

    def load(self, data=None, REQUEST=None):
        """ load replicated data """
        if data is None:
            data = REQUEST.get('data', None)
        if data is None:
            return
        payload = decompress(data)
        f = StringIO(payload)
        transaction = Unpickler(f).load()
        t_id = transaction['id']

        if t_id > self._last_transaction_id:
            self.REQUEST.set('replicating', '1')
            t = get_transaction()
            t.user = transaction['user'] + ' (replicated)'
            t.description = transaction['description']
            for entry_dict in transaction['entries'].values():
                self.realize_entry(entry_dict)
            zLOG.LOG('Replication', INFO, 'ReplicationLog.load> transaction id: %s' % oid2str(t_id)) #DBG 
        else:
            zlog('Replication', 
                 'Received old transaction: %s (last transaction: %s)'%(
                    t_id, self._last_transaction_id),
                 log_traceback=0)

    def _serialize_args(self, meth_args):

        def _serialize_value(value):
            """ Return a 'reference://' if value has a path.
                Otherwise, pass through.
            """
            if getattr(aq_base(value), 'getPhysicalPath', None):
                return 'reference://%s' % self._object_path(value)
            return value

        d = {}
        for k,v in meth_args.items():
            if not v:
                continue
            elif type(v) == type([]) or \
                    IReferenceListWrapper.isImplementedBy(v):
                d[k] = [_serialize_value(item) for item in v]
            elif isinstance(v, DateTime):
                d[k] = v.ISO()
            else:
                d[k] = _serialize_value(v)
        return d

    def _deserialize_args(self, meth_args):
        d = {}
        for k,v in meth_args.items():
            if type(v) == type([]):
                reference_list = []
                for item in v:
                    if hasattr(item, 'find') and \
                            item.startswith('reference://'):
                        path = self._siteRoot() + item[12:]
                        reference_list.append(self.unrestrictedTraverse(path))
                    else:
                        reference_list.append(item)
                d[k] = reference_list
            elif hasattr(v, 'find') and v.startswith('reference://'):
                path = self._siteRoot() + v[12:]
                d[k] = self.unrestrictedTraverse(path)
            elif type(v) == type(u''):
                d[k] = v.encode('ISO-8859-1') 
            elif type(v) == type(''):
                d[k] = unicode(v, 'ISO-8859-1').encode('ISO-8859-1')
            else:
                d[k] = v
        return d

    def realize_entry(self, entry_dict):
        # XXX: Temporary Hack to prevent 'realizing' of Reminders and
        # SystemEvents
        for mt in ('Reminder', 'SystemEvent'):
            if entry_dict['path'].split('/')[-1].find(mt) != -1:
                return
        path = self._siteRoot() + entry_dict['path']
        obj = self.unrestrictedTraverse(path)
        method = getattr(obj, entry_dict['method'])
        meth_args = self._deserialize_args(entry_dict['args'])
        method(**meth_args)

    def _object_path(self, obj):
        root = self.getCreme().getPhysicalPath()
        path = obj.getPhysicalPath()[len(root):]
        return '/'.join(path)

    def _siteRoot(self):
        return '/'.join(self.getCreme().getPhysicalPath(