marco () {
	echo "this is marco"
	MARCO_PWD=$(pwd)
}

polo () {
	echo "this is polo"
	if [ -n "$MARCO_PWD" ]; then
		echo "going back to $MARCO_PWD"
		cd "$MARCO_PWD" # quotes so paths with spaces work
	else
		echo "MARCO_PWD is not set"
	fi
}
