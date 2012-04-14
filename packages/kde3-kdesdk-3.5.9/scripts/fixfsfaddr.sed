#! /usr/bin/sed

# Copyright 2005 Nicolas GOUTTE <goutte@kde.org>
# License LGPL V2+

# The script helps to fix the FSF address
# Use:
#   find . -name .svn -prune , type f | xargs fgrep -l "Free Software Foundation" | xargs sed -i -f fixfsfaddr.sed
# Note: you should check the changes before committing them.

# Implementation note: we need to replace phrase by phrase, as 
# the wrapping of the FSF address is at different places.

# Current FSF address: 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301

# Old address: 59 Temple Place, Suite 330, Boston, MA  02111-1307
s/59 Temple Place,/51 Franklin Street,/
s/59 Temple Place -/51 Franklin Street,/
s/Suite 330,/Fifth Floor,/
s/Suite 330$/Fifth Floor/
s/02111-1307/02110-1301/

# Very old address: 675 Mass Ave, Cambridge, MA  02139
s/675 Mass Ave/51 Franklin Street, Fifth Floor/
s/Cambridge/Boston/
s/02139/02110-1301/
# Warning: the last two replaces seem to match the address of the MIT too.

# Typo in KDE: Franklin Steet
s/Franklin Steet/Franklin Street/
