import { useEffect, useState } from "react";
import { doc, onSnapshot, type DocumentData } from "firebase/firestore";
import { db } from "../firebase";

type HookResult<V> = {
  data: V | null;
  loading: boolean;
  error: Error | null;
};

export function useFirestoreDoc<T extends DocumentData>(
  collectionName: string,
  documentId: string,
): HookResult<T & { id: string }> {
  const [data, setData] = useState<(T & { id: string }) | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<Error | null>(null);

  useEffect(() => {
    if (!collectionName || !documentId) {
      setData(null);
      setError(null);
      setLoading(false);
      return;
    }

    setLoading(true);

    const ref = doc(db, collectionName, documentId);

    const unsubscribe = onSnapshot(
      ref,
      (snapshot) => {
        if (!snapshot.exists()) {
          setData(null);
          setError(new Error("Document does not exist"));
          setLoading(false);
          return;
        }

        setData({ id: snapshot.id, ...(snapshot.data() as T) });
        setError(null);
        setLoading(false);
      },
      (err) => {
        setError(err instanceof Error ? err : new Error(String(err)));
        setLoading(false);
      },
    );

    return unsubscribe;
  }, [collectionName, documentId]);

  return { data, loading, error };
}
