from vanido.model import track

if __name__ == '__main__':

    for id in track.listAllIds():
        print id
        t = track.getById(id)
        name = "(no name?)"
        try:
            name = t.name
        except:
            pass
        print " %s: \"%s\"" % (str(t), name)